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
        address, type = line.split(',')
        vpn = address[:-3]
        if type[0] == 'I':
            add_to_dict(instruction, vpn)
        else:
            add_to_dict(data, vpn)
            
    print('Instructions:')
    print_dict(instruction)
    print('Data:')
    print_dict(data)


if __name__ == '__main__':
    analyze(sys.argv[1])