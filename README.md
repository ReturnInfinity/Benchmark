Benchmarking programs and results

Apps in this repo:

- `l_bench.c` / `b_bench.asm` - Run `cpuid` 1000000 times. This should have similar results compared to running on Linux or BareMetal as `cpuid` is an "expensive" instruction.
- `l_ethernet_bench.c` / `b_ethernet_bench.asm` - Poll the network 1000000 times. The Linux version is pinned to a single CPU core to prevent additional delays. This isn't needed in BareMetal.

For network load testing [netflood](https://github.com/IanSeyler/netflood) was used. Both physical systems were wired directly to a 5-port 10Gbit NICGIGA switch (Model S100-0500T).

All Debian systems were installed without the "Debian desktop environment" and "GNOME". "SSH server" was added.

`apt install git gcc nasm`

# Virtual via KVM

Physical system for testing is as follows:

- Framework Desktop
- AMD RYZEN AI MAX+ 395 w/ Radeon™ 8060S × 32
- 128.0 GiB RAM
- Ubuntu 25.10

## l_bench / b_bench

Executing `cpuid` instruction in a loop.

### Linux (Debian 13.3.0)

```qemu-system-x86_64 -machine q35 -name "Debian 13.3.0" -smp sockets=1,cpus=4 -cpu host -enable-kvm -m 4096 -drive id=disk0,file=debian1330.img,if=none,format=raw -device virtio-scsi-pci -device scsi-hd,drive=disk0```

```
ian@debian-vm:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 2141.71 ns
ian@debian-vm:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 2143.63 ns
ian@debian-vm:~/Code/Testing$ ./l_bench
Iterations: 1000000
Average: 2129.82 ns
ian@debian-vm:~/Code/Testing$
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

### Linux (Debian 13.3.0)

Testing was done against the `virtio-net-pci` interface (enp0s4).

```qemu-system-x86_64 -machine q35 -name "Debian 13.3.0" -smp sockets=1,cpus=4 -cpu host -enable-kvm -m 4096 -drive id=disk0,file=debian1330.img,if=none,format=raw -device virtio-scsi-pci -device scsi-hd,drive=disk0 -netdev user,id=nat0 -device e1000,netdev=nat0 -netdev socket,id=priv0,listen=:12345 -device virtio-net-pci,netdev=priv0```

```
root@debian-vm:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp0s4 -n 1000000
Iterations: 1000000
Average: 107.87 ns
Bytes received: 0
root@debian-vm:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp0s4 -n 1000000
Iterations: 1000000
Average: 111.37 ns
Bytes received: 0
root@debian-vm:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp0s4 -n 1000000
Iterations: 1000000
Average: 108.34 ns
Bytes received: 0
root@debian-vm:/home/ian/Code/Benchmark$
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
- 240GB SATA (Kingston)
- Intel X540-T1 10Gbit network card (NICGIGA)
- Internal network adaptor disabled in BIOS

## l_bench / b_bench

### Linux (Debian 13.3.0)

```
ian@debian-amd:~/Code/Benchmark$ ./l_bench
Iterations: 1000000
Average: 28.51 ns
ian@debian-amd:~/Code/Benchmark$ ./l_bench
Iterations: 1000000
Average: 27.99 ns
ian@debian-amd:~/Code/Benchmark$ ./l_bench
Iterations: 1000000
Average: 28.11 ns
ian@debian-amd:~/Code/Benchmark$
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

### Linux (Debian 13.3.0)

#### No load

```
root@debian-amd:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 187.04 ns
Bytes received: 0
root@debian-amd:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 187.50 ns
Bytes received: 0
root@debian-amd:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 187.28 ns
Bytes received: 0
root@debian-amd:/home/ian/Code/Benchmark$
```

#### Load

```
root@debian-amd:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 262.24 ns
Bytes received: 322479000
root@debian-amd:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 261.95 ns
Bytes received: 322227000
root@debian-amd:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 262.30 ns
Bytes received: 322746000
root@debian-amd:/home/ian/Code/Benchmark$
```

### BareMetal (2026.01)

#### No load

```
> loadr
Enter file number: 5
> exec
Iterations: 1000000
Average: 3 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 3 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 3 ns
Bytes received: 0
>
```

#### Load

```
> loadr
Enter file number: 5
> exec
Iterations: 1000000
Average: 4 ns
Bytes received: 8650500
> exec
Iterations: 1000000
Average: 4 ns
Bytes received: 8598000
> exec
Iterations: 1000000
Average: 4 ns
Bytes received: 8581500
>
```

# Physical System (Intel)

Specs:
- [Intel® Core™ i5-12400](https://ark.intel.com/content/www/us/en/ark/products/134586/intel-core-i5-12400-processor-18m-cache-up-to-4-40-ghz.html) - Alder Lake - 6 cores, base 2.50GHz, boost 4.40GHz
- [ASUS PRIME B760M-A AX](https://www.asus.com/us/motherboards-components/motherboards/prime/prime-b760m-a-ax/)
- 16GiB RAM (1x 16GiB DDR5) - Max 128GiB
- 128GB NVMe (Patriot)
- Intel X540-T1 10Gbit network card (Beijing Sinead)
- Internal network adaptors disabled in BIOS

## l_bench / b_bench

### Linux (Debian 13.3.0)

```
ian@debian-intel:~/Code/Benchmark$ ./l_bench
Iterations: 1000000
Average: 32.17 ns
ian@debian-intel:~/Code/Benchmark$ ./l_bench
Iterations: 1000000
Average: 32.16 ns
ian@debian-intel:~/Code/Benchmark$ ./l_bench
Iterations: 1000000
Average: 32.42 ns
ian@debian-intel:~/Code/Benchmark$
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

### Linux (Debian 13.3.0)

#### No load

```
root@debian-intel:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 113.73 ns
Bytes received: 0
root@debian-intel:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 113.67 ns
Bytes received: 0
root@debian-intel:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 113.82 ns
Bytes received: 0
root@debian-intel:/home/ian/Code/Benchmark$
```

#### Load

```
root@debian-intel:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 146.13 ns
Bytes received: 179830500
root@debian-intel:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 147.71 ns
Bytes received: 181471500
root@debian-intel:/home/ian/Code/Benchmark$ ./l_ethernet_bench enp1s0 -n 1000000
Iterations: 1000000
Average: 148.75 ns
Bytes received: 182830500
root@debian-intel:/home/ian/Code/Benchmark$
```

### BareMetal (2026.01)

#### No load

```
> loadr
Enter file number: 5
> exec
Iterations: 1000000
Average: 3 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 3 ns
Bytes received: 0
> exec
Iterations: 1000000
Average: 3 ns
Bytes received: 0
>
```

#### Load

```
> loadr
Enter file number: 5
> exec
Iterations: 1000000
Average: 4 ns
Bytes received: 8181000
> exec
Iterations: 1000000
Average: 4 ns
Bytes received: 8188500
> exec
Iterations: 1000000
Average: 4 ns
Bytes received: 8184000
>
```

// EOF
