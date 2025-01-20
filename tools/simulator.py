import sys
import random
import serial

ser = serial.Serial(port="COM4", baudrate=19200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1)

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def checksum(data):
    c = 0
    for b in data:
        c += b
    c = (c & 0x0F) | 0x30
    return c.to_bytes(1, 'big')

def write(data):
    check = checksum(data)
    ser.write(data + check + b'$')
    ser.flush()
    eprint(b'write  ' + data + check + b'$')
    #sys.stdout.buffer.write(data + check + b'$')
    #sys.stdout.flush()

def write_plain(data):
    ser.write(data);
    ser.flush();
    eprint(b'write  ' + data)
    #sys.stdout.buffer.write(data)
    #sys.stdout.flush()

def read():
    data = b''
    while len(data) == 0:
        #data = sys.stdin.buffer.read(1)
        data = ser.read_all()
    eprint(b'read  ' + data)
    return data

def process_command():
    if read() == b'"':  # command prefix
        eprint("CMD Received")
        cmd = read()
        write_plain(cmd)
        if cmd == b'0':
            write(b'5321')  # version info
        elif cmd == b'?':
            if bool(random.getrandbits(1)):
                write(b':TTTTTTVVSMBBAXX')  # status update
            else:
                write(b'FBBBBBBBBG')
        else:
            write_plain(b'#')  # unknown command
    else:
        write_plain(b'#')  # if the command doesn't start with '"'

def main():
    # This loop will allow multiple commands to be processed.
    eprint("Simulator active")
    while True:
        try:
            process_command()
        except KeyboardInterrupt:
            # Gracefully handle termination
            break

if __name__ == "__main__":
    main()