package main

import (
	"fmt"
	"math/rand"
	"strings"
	"time"
)

func main() {
	loader(50)
}

func loader(col int) {

	for i := 0; i <= col; i++ {
		random := rand.Intn(200)
		if i == col {
			fmt.Printf("\r[%s]  %dms", strings.Repeat("=", col+1), random)
		} else {
			fmt.Printf("\r[%s>%s] %dms", strings.Repeat("=", i), strings.Repeat(" ", col-i), random)
		}
		time.Sleep(time.Duration(random) * time.Millisecond)
	}
	fmt.Println()
}
