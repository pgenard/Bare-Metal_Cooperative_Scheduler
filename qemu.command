./qemu/build/qemu-system-aarch64 -M imx8mp-evk -m 512M -kernel hello.bin -no-reboot -display none -serial stdio -monitor telnet:127.0.0.1:1234,server,nowait
