package main

import (
	"syscall"
	"unsafe"

	"github.com/gen2brain/shm"
)

func multiplyMatrixParallel(a, b [][]int, k int, c [][]int) {
	rows := len(a)
	if rows == 0 || len(b) == 0 || len(b[0]) == 0 || k <= 0 {
		return
	}

	cols := len(b[0])
	inner := len(b)
	if k > rows {
		k = rows
	}
	if rows%k != 0 {
		panic("rows must be divisible by k")
	}

	total := rows * cols
	bytesToShare := total * int(unsafe.Sizeof(int(0)))
	shmID, err := shm.Get(shm.IPC_PRIVATE, bytesToShare, shm.IPC_CREAT|0o600)
	if err != nil {
		panic(err)
	}
	defer shm.Ctl(shmID, shm.IPC_RMID, nil)

	mem, err := shm.At(shmID, 0, 0)
	if err != nil {
		panic(err)
	}
	defer shm.Dt(mem)

	if len(mem) < bytesToShare {
		panic("shared memory segment is smaller than expected")
	}

	shared := unsafe.Slice((*int)(unsafe.Pointer(&mem[0])), total)

	base := rows / k

	start := 0
	pids := make([]int, 0, k)
	for p := 0; p < k; p++ {
		chunk := base
		end := start + chunk

		pid, _, errno := syscall.RawSyscall(syscall.SYS_FORK, 0, 0, 0)
		if errno != 0 {
			panic(errno)
		}

		if pid == 0 {
			for i := start; i < end; i++ {
				for j := 0; j < cols; j++ {
					sum := 0
					for col := 0; col < inner; col++ {
						sum += a[i][col] * b[col][j]
					}
					shared[i*cols+j] = sum
				}
			}
			syscall.RawSyscall(syscall.SYS_EXIT, 0, 0, 0)
		}

		pids = append(pids, int(pid))

		start = end
	}

	for _, pid := range pids {
		var status syscall.WaitStatus
		_, err := syscall.Wait4(pid, &status, 0, nil)
		if err != nil {
			panic(err)
		}
	}

	for i := 0; i < rows; i++ {
		for j := 0; j < cols; j++ {
			c[i][j] = shared[i*cols+j]
		}
	}
}

