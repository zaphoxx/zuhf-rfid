#!/usr/bin/python3

import sys
import argparse
import random
import string
import serial
import time
import os


def main():
    logo()
    args = process_args(argparse.ArgumentParser())
    show_settings(args)
    #request, targetfilename = init(args)
    #check_response(request, args.basepath, targetfilename)
    input('<press \'return\' to start>\n')
    try:
      ser = serial.Serial(baudrate = args.baudrate, 
                          port = args.port,
                          timeout = args.timeout)
    except:
      print(F"[ERROR] Could not connect serial at {args.port}. Is device plugged in? Did you select the correct port?")
      #print(serial.tools.list_ports())
      sys.exit(1)
      
    time.sleep(2)
    
    ser.write(F'REP#{args.repetitions}#'.encode('latin-1'))
    ser.write(F'TXP#{args.tx_power}#'.encode('latin-1'))
    
    if (args.lock_flag):
      ser.write(F'LOCK#'.encode('latin-1'))
      # send data bits
    elif (args.write_flag):
      ser.write(F'WRITE#'.encode('latin-1'))
      ser.write(F"{args.mem_block}#{args.block_addr}#{args.n_words}#".encode('latin-1'))
      print(bytes.fromhex(args.data))
      ser.write(bytes.fromhex(args.data))
    elif (args.read_flag):
      ser.write(F'READ#'.encode('latin-1'))
      ser.write(F"{args.mem_block}#{args.block_addr}#{args.n_words}#".encode('latin-1'))
    else:
      ser.write(F'EPC#'.encode('latin-1'))
    
    # RUN SCAN #
    ser.write(b'DUMMY#') # terminate
    ser.write(b'RUN#')
    line = b""
    buffer = b""
    while line != b"#END":
      line = ser.readline().rstrip()
      if (line == b"#READDATA"):
        print(F"[MEM BLOCK {args.mem_block}]")
        print("#"*15)
        buffer = ser.read(args.n_words * 2)
        for i in range(args.n_words):
          #print(F"# {buffer.hex()}")
          print(F"{i+args.block_addr:#08x}: "+" ".join(format(x, "02x") for x in buffer[(i*2):(i*2)+2])) 
        print("#"*15)
      elif (line == b"#TAGDATA"):
        print("[TAG-DATA]");
        buffer = ser.read(16)
        print("#"*62)
        print(F"Stored PC: {' '.join(format(x,'#02x') for x in buffer[:2])}")
        print(F"EPC: {' '.join(format(x,'#02x') for x in buffer[2:14])}")
        print(F"CRC16: {' '.join(format(x,'#02x') for x in buffer[14:])}")
        print("#"*62)
      elif (line == b"WRITE#OK#"):
        print(F"# WRITE # {args.data} written to MEMBLOCK {args.mem_block} ADDR {args.block_addr} #") 
      elif (line == b"#DEBUG"):
        line = ser.readline().rstrip()
        print(line);
      else:
        print(line.decode('latin-1').ljust(80,' '),end='\r',flush=True)
    
    if ser.isOpen():
      ser.close()


def update_parameters(ser, args):
    
    time.sleep(1)

  
# process arguments    
def process_args(parser):
    parser.add_argument('-p', dest='port', help='serial port e.g. COM7,COM14', default='COM14')
    parser.add_argument('-b', dest='baudrate', type=int, help='baudrate of serial', default=250000)
    parser.add_argument('-tp', dest='tx_power', choices=[0x11,0x12,0x13,0x14,0x15,0x16],type=int,help='tx_power - index', default=0x16)
    parser.add_argument('-t', dest='timeout', help='serial read timeout', type=int, default=3)
    parser.add_argument('-r', dest='repetitions', help='repetitions to run before terminating',type=int, default=1000)
    #parser.add_argument('-read', dest = 'read_flag', action='store_true')
    parser.add_argument('-block', dest = 'block_addr', help='blockaddress to read/write from', type=int ,default=0)
    parser.add_argument('-mem', dest = 'mem_block', help='memory block to read / write from e.g. user block',choices=[0,1,2,3], type=int, default = 3)
    parser.add_argument('-n',dest = 'n_words', help='read n words from mem block, max is 32 words',type=int, default = 1)
    parser.add_argument('-data', dest = 'data', help='word data to write to mem block', default="1337")
    parser.add_argument('-write', dest = 'write_flag', action='store_true')
    parser.add_argument('-lock', dest = 'lock_flag', action='store_true')
    parser.add_argument('-read', dest = 'read_flag', action='store_true')
    #parser.print_help()
    args = parser.parse_args()
    if args.write_flag:
      response = input("[WARNING]You are about to potentially overwrite data!!!\n<continue y/n ?>")
      if response != 'y':
        sys.exit(0)
      if args.data == None:
        print("[ERROR] You need to provide a data word e.g. \"-data \'CAFE\'\" for writing")
        sys.exit(0)
    if args.n_words > 8:
      print("[WARNING] -n values larger then 8 might cause unreliable read/writes! It is recommended to use a maximum of 8 words for read/write")
      resp = input("<continue y/n?>")
      if resp != "y":
        sys.exit(1)
    return args

def show_settings(args):
    print('[info] Using following settings:')
    print('-'*35)
    for k,v in args.__dict__.items():
        print(F'{str(k).ljust(10)}: {str(v).rjust(20)}')
    print('-'*35)


def logo():
  print();
  print(" @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@\n      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@\n    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!\n  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!\n :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : \n");
  print();

if __name__ == '__main__':
  main()
  

