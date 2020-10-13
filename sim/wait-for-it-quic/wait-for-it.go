package main

import (
	"bytes"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"time"
)

// compose a packet that will elicit a Version Negotiation packet
var packet = append([]byte{0xc0, 0x57, 0x41, 0x49, 0x54, 0, 0}, make([]byte, 1200)...)

func main() {
	startTime := time.Now()
	var timeout time.Duration
	flag.DurationVar(&timeout, "t", 0, "timeout (e.g. 10s)")
	flag.Parse()
	host := flag.Arg(0)
	if flag.NArg() < 1 {
		flag.Usage()
		return
	}
	if timeout == 0 {
		fmt.Printf("waiting for %s without a timeout\n", host)
	} else {
		fmt.Printf("waiting %s for %s\n", timeout, host)
	}
	addr, err := net.ResolveUDPAddr("udp", host)
	if err != nil {
		log.Fatal(err)
	}
	ok := waitForIt(addr, timeout)
	dur := time.Since(startTime)
	if !ok {
		fmt.Printf("timeout occurred after waiting %s for %s\n", dur, host)
		os.Exit(1)
	}
	fmt.Printf("%s is available after %s\n", host, dur)
}

func waitForIt(addr *net.UDPAddr, timeout time.Duration) bool {
	conn, err := net.ListenUDP("udp", nil)
	if err != nil {
		log.Fatal("ListenUDP failed: %s", err)
	}
	done := make(chan struct{})
	go func() {
		b := make([]byte, 1500)
		n, err := conn.Read(b)
		if err != nil {
			log.Fatalf("Read failed: %s", err)
			return
		}
		b = b[:n]
		if len(b) < 5 {
			log.Fatal("invalid packet")
		}
		if !bytes.Equal(b[1:5], []byte{0, 0, 0, 0}) {
			log.Fatal("expected Version Negotiation packet")
		}
		close(done)
	}()

	ticker := time.NewTicker(time.Second / 2)
	var timerChan <-chan time.Time
	if timeout > 0 {
		timer := time.NewTimer(timeout)
		defer timer.Stop()
		timerChan = timer.C
	}
	for {
		if _, err := conn.WriteTo(packet, addr); err != nil {
			log.Fatalf("Write failed: %s", err)
			return false
		}
		select {
		case <-ticker.C:
		case <-timerChan:
			return false
		case <-done:
			return true
		}
	}
}
