package main

import (
	"context"
	"fmt"
	"math/rand"
	"os"
	"os/signal"
	"strings"
	"sync"
	"syscall"
	"time"
)

var mut sync.Mutex

func main() {
	var wg sync.WaitGroup
	const col = 50
	const numLoaders = 10

	// Create a context that can be cancelled
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// Setup signal handling
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, os.Interrupt, syscall.SIGTERM)

	// Handle signals in a separate goroutine
	go func() {
		<-sigChan
		cancel() // Cancel context to stop all loaders
	}()

	// Reserve space by printing newlines
	for range numLoaders {
		fmt.Println()
	}

	// Save cursor position and move up to reserved space
	fmt.Printf("\033[%dA", numLoaders) // Move cursor up

	for i := range numLoaders {
		wg.Add(1)
		go func(lineNum int) {
			defer wg.Done()
			loader(ctx, col, lineNum)
		}(i + 1)
	}
	wg.Wait()

	// Move cursor below all progress bars
	fmt.Printf("\033[%dB", numLoaders)

	select {
	case <-ctx.Done():
		fmt.Println("Loaders interrupted!")
	default:
		fmt.Println("All loaders completed!")
	}
}

func loader(ctx context.Context, col, lineNum int) {
	for i := 0; i <= col; i++ {
		// Check if context is cancelled
		select {
		case <-ctx.Done():
			return
		default:
			// Continue normal operation
		}

		random := rand.Intn(200)

		mut.Lock()
		// Save current cursor position, move to specific line, print, restore position
		fmt.Printf("\033[s")              // Save cursor position
		fmt.Printf("\033[%dB", lineNum-1) // Move down to target line
		fmt.Printf("\r\033[K")            // Move to start and clear line
		if i == col {
			fmt.Printf("Loader %d: [%s] ✓ %dms", lineNum, strings.Repeat("=", col+1), random)
		} else {
			fmt.Printf("Loader %d: [%s>%s] %dms", lineNum, strings.Repeat("=", i), strings.Repeat(" ", col-i), random)
		}
		fmt.Printf("\033[u") // Restore cursor position
		mut.Unlock()

		time.Sleep(time.Duration(random) * time.Millisecond)
	}
}
