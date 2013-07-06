'''
Created on Apr 18, 2011

@author: mike-bowles
'''
from mrjob.job import MRJob
from mrjob.emr import EMRJobRunner
from random import sample
import simplejson as json
from boto.s3.connection import S3Connection
from boto.s3.key import Key

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
            '--k', dest='kMeans', default=2, type='int',
            help='k: number of means (cluster centroids)')
        #self.add_passthrough_option(
        #    '--pathName', dest='pathName', default="//home//mike-bowles//pyWorkspace//mapReducers//src//kMeans3//", type='str',
        #    help='pathName: pathname where intermediateResults.txt is stored')
        self.add_passthrough_option(
            '--pathName', dest='pathName', default="s3://mike-mrjob/kMeans/centroids/intermediateResults.txt", type='str',
            help='pathName: pathname where intermediateResults.txt is stored')
    def mapper(self, key, xjIn):
        #something simple to grab random starting point
        #collect the first 2k
        if self.count <= 2*self.options.kMeans:
            self.count += 1
            yield (1,xjIn)        
        
    def reducer(self, key, xjIn):        
        #accumulate data points mapped to 0 from 1st mapper and pull out k of them as starting point
        #AWS_Access_Key and AWS_Secret_Access_Key
        conn = S3Connection(AWS_Access_Key, AWS_Secret_Access_Key)
        pb = conn.get_bucket('mike-mrjob')
        k = Key(pb)
        k.name = "kMeans/centroids/intermediateResults.txt"
        
        cent = []
        for xj in xjIn:
            x = json.loads(xj)
            cent.append(x)
            yield 1, xj
        index = sample(range(len(cent)), self.options.kMeans)
        cent2 = []
        for i in index:
            cent2.append(cent[i])
               
        centOut = json.dumps(cent2)
        #put centroids onto file to load in at next instantiation of "self"
        #key = EMRJobRunner().get_s3_key(self.options.pathName)
        #key.set_contents_from_string(centOut)
        k.set_contents_from_string(centOut)
        
if __name__ == '__main__':
    MRkMeansInit.run()