'''
Created on Apr 18, 2011

@author: mike-bowles
'''
from mrSample import mrSample
import simplejson as json
from math import sqrt
import os


PROJECT_ROOT = os.path.dirname(os.path.realpath(__file__))



def main():
    #first run the initializer to get starting centroids
    filePath =  os.path.join(PROJECT_ROOT, 'input.txt')
    mrJob = mrSample(args=[filePath])
    with mrJob.make_runner() as runner:
        runner.run()

    x = 3






if __name__ == '__main__':
    main()