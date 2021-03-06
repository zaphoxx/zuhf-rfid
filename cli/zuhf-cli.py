#!/usr/bin/python3

import sys
import argparse
import random
import string
import serial
import time
import os
from colorama import Fore, Back, Style


def main():
    logo()
    test = False
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
    
    
    '''
        About Access:
        If a access password is provided it will trigger the access sequence before
        reading or writing data from/to tag.
        An access password which is "00000000" is for the tag effectively as if no
        password has been set.
        The tag will respond to an req_rn request and transission into "secured" state
        if no password is set or into "open" state if password has been set. In the later
        case the access sequence needs to be performed to transission the tag to the
        "secured" state.
    '''
    if (args.access_pwd != "00000000"):
        lpass = args.access_pwd[:4]
        hpass = args.access_pwd[4:]
        print(lpass, hpass)
        if (args.write_flag):
            ser.write(b"WRITEACCESS#" + \
            bytes.fromhex(lpass) + \
            b"#" + bytes.fromhex(hpass) + b"#" + \
            F"{args.mem_block}#{args.block_addr}#{args.n_words}#".encode('latin-1') + \
            bytes.fromhex(args.data) + b"#")
        if (args.read_flag):
            ser.write(b"READACCESS#" + \
            bytes.fromhex(lpass) + b"#" + \
            bytes.fromhex(hpass) + b"#" + \
            F"{args.mem_block}#{args.block_addr}#{args.n_words}#".encode('latin-1') + b"#")
        if (args.lock_flag):
            print(Fore.CYAN + "### LOCKACCESS ###")
            mask_bytes = bytes([int(x) for x in args.lock_mask])
            action_bytes = bytes([int(x) for x in args.lock_action])
            ser.write(b"LOCKACCESS#" + \
            bytes.fromhex(lpass) + b"#" + \
            bytes.fromhex(hpass) + b"#" + \
            mask_bytes + action_bytes + b"#")
    
    elif (args.write_flag):
      #ser.write(F'WRITE#'.encode('latin-1'))
      ser.write(b"WRITE#" + F"{args.mem_block}#{args.block_addr}#{args.n_words}#".encode('latin-1') + bytes.fromhex(args.data) + b"#")
      print(bytes.fromhex(args.data))
      #ser.write(bytes.fromhex(args.data)+b"#")
      
    elif (args.read_flag):
      #ser.write(F'READ#'.encode('latin-1'))
      ser.write(b"READ#"+F"{args.mem_block}#{args.block_addr}#{args.n_words}#".encode('latin-1'))
    elif (args.lock_flag):
      mask_bytes = bytes([int(x) for x in args.lock_mask])
      action_bytes = bytes([int(x) for x in args.lock_action])
      ser.write(b"LOCK#" + mask_bytes + action_bytes + b"#")
      #ser.write(F'LOCK#'.encode('latin-1'))
      #ser.write(mask_bytes)  
      #ser.write(action_bytes)
      #ser.write(b"#")
    elif (args.monza):
      ser.write(b"MONZA#")          
      
    else:
      ser.write(b"READ_EPC#")
    
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
          print(Fore.CYAN + F"{i+args.block_addr:#08x}: "+" ".join(format(x, "02x") for x in buffer[(i*2):(i*2)+2]))
        print(Fore.WHITE + "#"*15)
      elif (line == b"#TAGDATA"):
        print(Fore.GREEN + "[TAG-DATA]")
        buffer = ser.read(16)
        print("#"*62)
        print(F"Stored PC: ".ljust(14) + F"{' '.join(format(x,'#02x') for x in buffer[:2])}")
        epc = ''.join('{0:0{1}X}'.format(x,2) for x in buffer[2:14])
        print(F"EPC: ".ljust(14) + F"{epc}")
        print(F"CRC16: ".ljust(14) + F"{' '.join(format(x,'#02x') for x in buffer[14:])}")
        print("#"*62 + Fore.WHITE)
        
      elif (line == b"WRITE#OK#"):
        print(Fore.GREEN + F"[+] WRITE COMPLETE" + Fore.WHITE)
      elif (line == b"#DEBUG"):
        line = ser.readline().rstrip()
        print(Fore.BLUE + F"{line.decode('latin-1')}"+ Fore.WHITE)
      elif (line == b"#ERROR"):
        line = ser.readline().rstrip()
        print(Fore.RED + F"{line.decode('latin-1')}" + Fore.WHITE)
        
      else:
        pass
        #print(line.decode('latin-1').ljust(80,' '),end='\r',flush=True)
    
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
    
    # arguments used in combination with actions read/write (-data is only relevant for the write action)
    parser.add_argument('-block', dest = 'block_addr', help='blockaddress to read/write from', type=int ,default=0)
    parser.add_argument('-mem', dest = 'mem_block', help='memory block to read / write from e.g. user block',choices=[0,1,2,3], type=int, default = 3)
    parser.add_argument('-n',dest = 'n_words', help='read n words from mem block, max is 32 words',type=int, default = 1)
    parser.add_argument('-data', dest = 'data', help='word data to write to mem block', default="1337")
    
    # arguments for -lock action
    parser.add_argument('-mask', dest = 'lock_mask', default='0000000000')
    parser.add_argument('-action', dest = 'lock_action', default = '0000000000')
    
    # access / authentication actions    
    parser.add_argument('-access', dest = 'access_pwd', default='00000000')

    # available actions: write,read,lock (mutually exclusive)   
    action = parser.add_mutually_exclusive_group()
    action.add_argument('-write', dest = 'write_flag', action='store_true')
    action.add_argument('-lock', dest = 'lock_flag', action='store_true')
    action.add_argument('-read', dest = 'read_flag', action='store_true')
    action.add_argument('-monza', dest = 'monza', action='store_true')
    
    #parser.print_help()
    args = parser.parse_args()
    
    if args.write_flag:
      if args.data == None:
        print("[ERROR] You need to provide a data word e.g. \"-data \'CAFE\'\" for writing")
        sys.exit(0)
      # safety check ; truncate data to align with word length and set n appropriately
      args.data = args.data[:len(args.data)-len(args.data)%4]
      args.n_words = len(args.data)//4
      print(args.data, args.n_words)
    return args

def show_settings(args):
    print('[info] Using following settings:')
    print('-'*37)
    for k,v in args.__dict__.items():
        print(F'{str(k).ljust(15)}: {str(v).rjust(20)}')
    print('-'*37)


def logo():
  print();
  print(" @@@@@@@@ @@@  @@@ @@@  @@@ @@@@@@@@      @@@@@@@  @@@@@@@@ @@@ @@@@@@@\n      @@! @@!  @@@ @@!  @@@ @@!           @@!  @@@ @@!      @@! @@!  @@@\n    @!!   @!@  !@! @!@!@!@! @!!!:!        @!@!!@!  @!!!:!   !!@ @!@  !@!\n  !!:     !!:  !!! !!:  !!! !!:           !!: :!!  !!:      !!: !!:  !!!\n :.::.: :  :.:: :   :   : :  :             :   : :  :       :   :: :  : \n");
  print();

if __name__ == '__main__':
  main()
  

