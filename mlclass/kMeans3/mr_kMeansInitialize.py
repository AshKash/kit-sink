'''
Created on Apr 18, 2011

@author: mike-bowles
'''
from mrjob.job import MRJob


import random
import simplejson as json

class MRkMeansInit(MRJob):
    DEFAULT_PROTOCOL = 'json'
    
    def __init__(self, *args, **kwargs):
        super(MRkMeansInit, self).__init__(*args, **kwargs)
        self.centroids = []                  #current centroid list
        self.numMappers = 1             #number of mappers
        self.count = 0
        
                                                 
    def configure_options(self):
        super(MRkMeansInit, self).configure_options()
        self.add_passthrough_option(
            '--k', dest='kMeans', type='int',
            help='k: number of means (cluster centroids)')
        self.add_passthrough_option(
            '--pathName', dest='pathName', default="/Users/ashwin/repos/mlclass/kMeans3/", type='str',
            help='pathName: pathname where intermediateResults.txt is stored')
        
        
        
        
    def mapper(self, key, xjIn):
        #something simple to grab random starting point
        #collect the first 2k
        if self.count <= 2*self.options.kMeans:
            self.count += 1
            print (1,xjIn)
            yield (1,xjIn)        
        
    def reducer(self, key, xjIn):        
        #accumulate data points mapped to 0 from 1st mapper and pull out k of them as starting point
        cent = []
        for xj in xjIn:
            x = json.loads(xj)
            cent.append(x)
            yield 1, xj
        index = random.sample(range(len(cent)), self.options.kMeans)
        cent2 = []
        for i in index:
            cent2.append(cent[i])
               
        print cent2
        centOut = json.dumps(cent2)
        #put centroids onto file to load in at next instantiation of "self"
        fullPath = self.options.pathName + 'intermediateResults.txt'
        fileOut = open(fullPath,'w')
        fileOut.write(centOut)
        fileOut.close()

if __name__ == '__main__':
    MRkMeansInit.run()
