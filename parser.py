import os
import re
import argparse

from tabulate import tabulate
import matplotlib.pyplot as plt


class Parser:
    '''
    Base Parser class providing common methods for all parsers
    '''
    def __init__(self, file, suffix):
        self.file = file
        self.lines = []
        self.rows = []
        self.data_set = []
        self.headers = []

        self.suffix = suffix
        self.readFile()
        self.parse()

    ''' handling log file and reading it as line list to self.lines '''
    def readFile(self):
        if os.path.exists(self.file):
            if self.checkSuffix():
                with open(self.file, 'r') as f:
                    self.lines = f.readlines()
            else:
                raise AttributeError('File extension not match parser extension')
        else:
            raise AttributeError('File not exist')

    ''' check if suffix of file is good '''
    def checkSuffix(self):
        if self.suffix is not None:
            return self.file.split('.')[-1] == self.suffix

    ''' main parser method, have to set self.headers and self.data_set after parsing '''
    def parse(self):
        pass

    ''' method to display data, by default display self.data_set '''
    def display(self, data=None):
        if data is None:
            data = self.data_set

        print(tabulate(data, headers=self.headers, tablefmt='plain'))

    ''' 
    method for filtering data_set by one parameter, supports three methods ('=', '<', '>') 
    param have to be in headers 
    '''
    def filter(self, param, method, value, data=None):
        if data is None:
            data = self.data_set

        out_data = []
        if param in self.headers:
            index = self.headers.index(param)
            for row in data:
                if method == '=':
                    if row[index] == value:
                        out_data.append(row)
                elif method == '>':
                    if row[index] > value:
                        out_data.append(row)
                elif method == '<':
                    if row[index] < value:
                        out_data.append(row)

        else:
            raise AttributeError(f'Could not find {param} in headers')

        return out_data



class TrData:
    ''' Class for data model of one line of TR log '''

    header = ('type', 'time', 'protocol', 'send_addr', 'reci_addr', 'length')
    # fixed headers for TrDataRow

    def __init__(self):
        self.line = None
        self.event_type = None
        self.event_time = 0
        self.header = None
        self.sender_addr = None
        self.receiver_addr = None
        self.node = None
        self.device = None
        self.length = 0

    ''' Method for list representation of object '''
    def getRowList(self):
        return (self.event_type, self.event_time, self.header, self.sender_addr, self.receiver_addr, self.length)

    ''' Method for string representation of object '''
    def __str__(self):
        return f'{self.event_type} {self.header} {self.sender_addr} {self.receiver_addr}\t {self.node} {self.device} {self.length}'


class TxtParser(Parser):
    def __init__(self, file):
        Parser.__init__(self, file, 'txt')

    ''' parse implementation for 'txt' files '''
    def parse(self):

        header_text = self.lines[0]
        self.headers = header_text.strip().split('\t')
        self.headers[0] = self.headers[0][2:]  # cutting '% ' at first header


        for line in self.lines[1:]:
            values = line.strip().split('\t')
            for n, value in enumerate(values):
                try:
                    value = float(value)
                    if value.is_integer():
                        value = int(value)
                except:
                    pass
                values[n] = value
            self.data_set.append(values)


class TrParser(Parser):

    def __init__(self, file):
        Parser.__init__(self, file, 'tr')
        self.headers = TrData.header

    ''' method for adding parsed data to data_set '''
    def addRecord(self, line, event_type, event_time, header, addr, node, device, length):
        data = TrData()
        data.line = line
        data.event_type = event_type
        data.event_time = float(event_time)
        data.header = header
        data.node = node
        data.device = device
        data.length = length
        if addr is not None:
            addr = addr.group()
            data.sender_addr, data.receiver_addr = addr.strip().split(' > ')

        self.data_set.append(data.getRowList())
        self.rows.append(data)

    ''' 
    method for plotting graph based on network load to time
    quantum <float> is time to slice whole timeline and in that parts sum all load
    '''
    def plot(self, quantum=0.01):
        t = 0
        le = 0

        x = []
        y = []
        for data in self.rows:
            le += data.length
            if data.event_time != t:
                x.append(t)
                y.append(le)
                le = 0
                t = data.event_time


        fig, ax = plt.subplots()
        ax.plot(x, y)
        ax.grid()
        plt.show()

    ''' method for counting size of whole pack (headers and payload) '''
    def countLength(self, headers):
        length = 0
        for n in range(3):
            if 'length: ' in headers[n][1]:
                length += int(headers[n][1].split('length: ')[1].split(' ')[0].strip())
            if 'Payload: ' in headers[n][1]:
                length += int(headers[n][1].split('Payload (size=')[1].split(')')[0].strip())
        return length

    ''' parse implementation for TR files'''
    def parse(self):
        pattern = r'[0-9]+[.][0-9]+[.][0-9]+[.][0-9][ ][>][ ][0-9]+[.][0-9]+[.][0-9]+[.][0-9]'

        lines = self.lines
        for line in lines:
            words = line.split(' ')

            event_type = words[0]
            event_time = words[1]

            node = words[2].split('/')[2]
            device = words[2].split('/')[4]

            if event_type in ('+', '-'):
                words = line.strip().split('ns3::')
                headers = []
                for word in words:
                    if 'Header ' in word:
                        h_type, value = word.split('Header ')
                        h_type = h_type + 'Header'
                        if h_type != 'PppHeader':
                            headers.append((h_type, value))

                h_len = len(headers)
                if h_len in (3, 6):
                    header = headers[2][0]
                    addr = re.search(pattern, headers[0][1])
                    length = self.countLength(headers)

                    self.addRecord(line, event_type, event_time, header, addr, node, device, length)

                    if h_len == 6:
                        header = headers[5][0]
                        addr = re.search(pattern, headers[3][1])
                        length = self.countLength(headers)

                        self.addRecord(line, event_type, event_time, header, addr, node, device, length)



def plotTest():
    parser = TrParser('dane/log.tr')
    parser.plot()


def parseAllInDataDir():
    import glob

    for file in glob.glob("dane/*.txt"):
        print(file)
        parser = TxtParser(file)
        parser.display()


def filterTest():
    parser = TxtParser('dane/DlMacStats.txt')
    data = parser.filter('RNTI', '=', 1)
    parser.display(data)


def run():
    parser = argparse.ArgumentParser(description='Parser for ns-3 log files, support (*.txt, *.tr)')
    parser.add_argument('file')
    parser.add_argument("-CELLID", help="filter by Cellid value", type=int, action="store")
    parser.add_argument("-IMSI", help="filter by IMSI value", type=int, action="store")
    parser.add_argument("-RNTI", help="filter by RNTI value", type=int, action="store")
    parser.add_argument("-plot", help="plot *.tr ", action="store_true")
    args = parser.parse_args()


    if args.file.endswith('.txt'):
        parser = TxtParser(args.file)
    elif args.file.endswith('.tr'):
        parser = TrParser(args.file)
    else:
        e = args.file.split('.')[-1]
        raise AttributeError(f'{e} is not in (txt, tr) unknown to parse')

    if args.plot:
        if isinstance(parser, TrParser):
            parser.plot()
        else:
            raise AttributeError('-plot only enable with tr logs')
    else:
        data = None

        if args.CELLID is not None:
            data = parser.filter('cellId', '=', args.CELLID)
        if args.IMSI is not None:
            data = parser.filter('IMSI', '=', args.IMSI, data)
        if args.RNTI is not None:
            data = parser.filter('RNTI', '=', args.RNTI, data)

        parser.display(data)


if __name__ == '__main__':
    run()
