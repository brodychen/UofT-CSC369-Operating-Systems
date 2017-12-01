#!/usr/bin/python

if __name__ == '__main__':
	with open("FILE_ONEBLK.txt", 'w') as f:
		f.write('ONEBLOCKONEBLOCK'*(1024//16))

