from pwn import remote, log, p8, sleep, u32

p = remote("78.141.226.4", 54321)

with open("./test_example", "rb") as f:
    data = f.read()
log.info(f"Sending data: {data}")
for i in range(len(data)):
    p.send(p8(data[i]))
out = int.from_bytes(p.recv(), byteorder='big')
log.info(f"Server returned: {out}")
