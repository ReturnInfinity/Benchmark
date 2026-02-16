Benchmarking programs and results

# Virtual via KVM

Physical system for testing is as follows:

- Framework Desktop
- AMD RYZEN AI MAX+ 395 w/ Radeon™ 8060S × 32
- 128.0 GiB RAM
- Ubuntu 25.10

## l_bench / b_bench

Executing `cpuid` instruction in a loop.

### Linux (Ubuntu 25.10)

```qemu-system-x86_64 -machine q35 -name "Ubuntu 25.10" -smp sockets=1,cpus=4 -cpu host -enable-kvm -m 4096 -drive id=disk0,file=ubuntu2510.img,if=none,format=raw -device virtio-scsi-pci -device scsi-hd,drive=disk0```

```
ian@ubuntu2510:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 2270.05 ns
ian@ubuntu2510:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 2269.85 ns
ian@ubuntu2510:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 2286.01 ns
ian@ubuntu2510:~/Code/Testing$
```

### BareMetal (2026.01)

```qemu-system-x86_64 -machine q35 -name "BareMetal OS" -smp sockets=1,cpus=4 -cpu host -enable-kvm -m 256 -drive id=disk0,file="sys/baremetal_os.img",if=none,format=raw -device ide-hd,drive=disk0```

```
> load
Enter file number: 4
> exec
Iterations: 1000000
Average: 2231 ns
> exec
Iterations: 1000000
Average: 2236 ns
> exec
Iterations: 1000000
Average: 2217 ns
>
```

### Summary

This verified that the benchmarking tool is working correctly on both Linux and BareMetal OS.

## l_ethernet_bench / b_ethernet_bench

Executing relevant function for reading Ethernet packets.

### Linux (Ubuntu 25.10)

Testing was done against the `virtio-net-pci` interface (enp0s4).

```qemu-system-x86_64 -machine q35 -name "Ubuntu 25.10" -smp sockets=1,cpus=4 -cpu host -enable-kvm -m 4096 -drive id=disk0,file=ubuntu2510.img,if=none,format=raw -device virtio-scsi-pci -device scsi-hd,drive=disk0 -netdev user,id=nat0 -device e1000,netdev=nat0 -netdev socket,id=priv0,listen=:12345 -device virtio-net-pci,netdev=priv0```

```
ian@ubuntu2510:~/Code/Testing$ sudo ./l_ethernet_bench enp0s4 -n 1000000
Iterations: 1000000
Average: 120.82 ns
Bytes received: 0
ian@ubuntu2510:~/Code/Testing$ sudo ./l_ethernet_bench enp0s4 -n 1000000
Iterations: 1000000
Average: 121.37 ns
Bytes received: 0
ian@ubuntu2510:~/Code/Testing$ sudo ./l_ethernet_bench enp0s4 -n 1000000
Iterations: 1000000
Average: 120.36 ns
Bytes received: 0
ian@ubuntu2510:~/Code/Testing$
```

### BareMetal (2026.01)

```qemu-system-x86_64 -machine q35 -name "BareMetal OS" -smp sockets=1,cpus=4 -cpu host -enable-kvm -m 256 -drive id=disk0,file="sys/baremetal_os.img",if=none,format=raw -device ide-hd,drive=disk0 -netdev socket,id=testnet1,connect=localhost:12345 -device virtio-net-pci,netdev=testnet1,mac=10:11:12:00:1A:F4```

```
> load
Enter file number: 5
> exec
Iterations: 1000000
Average: 13 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 14 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 13 ns
Bytes received: 0
>
```

# Physical System (AMD)

Specs:
- [AMD Ryzen 7 7700X](https://www.amd.com/en/products/processors/desktops/ryzen/7000-series/amd-ryzen-7-7700x.html) - Zen 4 (Raphael) - 8 cores, base 4.50GHz, boost 5.40GHz
- [ASUS PRIME B650M-A II](https://www.asus.com/motherboards-components/motherboards/csm/prime-b650m-a-ii-csm/)
- 16GiB RAM (1x 16GiB DDR5) - Max 128GiB
- 128GB NVMe (Patriot)
- Intel X540-T1 10Gbit network card (NICGIGA)
- Internal network adaptor disabled in BIOS

## l_bench / b_bench

### Linux (Ubuntu 25.10)

```
ian@amd:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 31.20 ns
ian@amd:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 31.20 ns
ian@amd:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 31.21 ns
ian@amd:~/Code/Testing$
```

### BareMetal (2026.01)

```
> loadr
Enter file number: 4
> exec
Iterations: 1000000
Average: 27 ns
> exec
Iterations: 1000000
Average: 27 ns
> exec
Iterations: 1000000
Average: 27 ns
>
```

## l_ethernet_bench / b_ethernet_bench

### Linux (Ubuntu 25.10)

```
ian@amd:~/Code/Testing$ sudo ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 389.97 ns
Bytes received: 0
ian@amd:~/Code/Testing$ sudo ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 390.19 ns
Bytes received: 0
ian@amd:~/Code/Testing$ sudo ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 390.24 ns
Bytes received: 0
ian@amd:~/Code/Testing$
```

### BareMetal (2026.01)

```
> loadr
Enter file number: 5
> exec
Iterations: 1000000
Average: 1 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 2 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 1 ns
Bytes received: 0
>
```

# Physical System (Intel)

Specs:
- [Intel® Core™ i5-12400](https://ark.intel.com/content/www/us/en/ark/products/134586/intel-core-i5-12400-processor-18m-cache-up-to-4-40-ghz.html) - Alder Lake - 6 cores, base 2.50GHz, boost 4.40GHz
- [ASUS PRIME B760M-A AX](https://www.asus.com/us/motherboards-components/motherboards/prime/prime-b760m-a-ax/)
- 16GiB RAM (1x 16GiB DDR5) - Max 128GiB
- Intel X540-T1 10Gbit network card (Beijing Sinead)
- Internal network adaptors disabled in BIOS

## l_bench / b_bench

### Linux (Ubuntu 25.10)

```
ian@intel:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 33.14 ns
ian@intel:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 32.01 ns
ian@intel:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 32.64 ns
ian@intel:~/Code/Testing$
```

### BareMetal (2026.01)

```
> loadr
Enter file number: 4
> exec
Iterations: 1000000
Average: 31 ns
> exec
Iterations: 1000000
Average: 31 ns
> exec
Iterations: 1000000
Average: 31 ns
>
```

## l_ethernet_bench / b_ethernet_bench

### Linux (Ubuntu 25.10)

```
ian@intel:~/Code/Testing$ sudo ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 126.76 ns
Bytes received: 0
ian@intel:~/Code/Testing$ sudo ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 123.26 ns
Bytes received: 0
ian@intel:~/Code/Testing$ sudo ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 124.14 ns
Bytes received: 0
ian@intel:~/Code/Testing$
```

### BareMetal (2026.01)

```
> loadr
Enter file number: 5
> exec
Iterations: 1000000
Average: 1 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 1 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 1 ns
Bytes received: 0
>
```


// EOF
