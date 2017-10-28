import sys

def analyze(file):
    
    # File to analyze
    f = open(file, 'r')

    # Count data and instruction separately
    data = dict()
    instruction = dict()

    # Add to dict
    def add_to_dict(d, vpn):
        if vpn in d:
            d[vpn] += 1
        else:
            d[vpn] = 1
    
    def print_dict(d):
        for vpn, number in sorted(d.items(), key = lambda a : a[1], reverse = True):
            print(vpn + ',', end = '')
            print(number)

    for line in f.readlines():
        type, address = line.split()
        vpn = address[:-3]
        if type[0] == 'I':
            add_to_dict(instruction, vpn)
        else:
            add_to_dict(data, vpn)
    
    len_instruction = len(instruction)
    len_data = len(data)
    print('# Total pages:\t', len_instruction + len_data)
    print('# Instruction pages:\t', len_instruction)
    print_dict(instruction)
    print('# Data pages:\t', len_data)
    print_dict(data)


if __name__ == '__main__':
    analyze(sys.argv[1])