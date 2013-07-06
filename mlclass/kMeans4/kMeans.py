'''
Created on Apr 18, 2011

@author: mike-bowles
'''
from mrjob.emr import EMRJobRunner
from mr_kMeansInitialize import MRkMeansInit
from mr_kMeansIterate import MRkMeansIter
import simplejson as json
from math import sqrt
#import boto

'''
This is a calling program to run several mr jobs
1.  first it calls an mrjob to run through the data set and pick k random points as starting points for iteration
2.  control iterative process by stepping through iterative improvements in centroid calculations until convergence
Improvements - 
a.  more orderly handling of path to intermediate results
b.  really random selection of starting inputs
c.  calculation of SSE
d.  spread reducer calc over multiple reducers. 

'''


def dist(x,y):
    #euclidean distance between two lists    
    sum = 0.0
    for i in range(len(x)):
        temp = x[i] - y[i]
        sum += temp * temp
    return sqrt(sum)


def main():
    #first run the initializer to get starting centroids
    filePath = '/home/mike-bowles/pyWorkspace/mapReducers/src/kMeans3/input.txt'
    
    
    #mrJob = MRkMeansInit(args=[filePath])
    #mrJob = MRkMeansInit(args=['-r', 'emr', filePath])
    #with mrJob.make_runner() as runner:
    #    runner.run()
    
    #pull out the centroid values to compare with values after one iteration
    centPath = "s3://mike-mrjob/kMeans/centroids/intermediateResults.txt"
    key = EMRJobRunner().get_s3_key(centPath)
    centroidsJson = key.get_contents_as_string()
    
    
    delta = 10
    #Begin iteration on change in centroids
    while delta > 0.01:
        #parse old centroid values
        oldCentroids = json.loads(centroidsJson)
        #run one iteration
        mrJob2 = MRkMeansIter(args=['-r', 'emr', filePath])
        with mrJob2.make_runner() as runner:
            runner.run()
            
        #compare new centroids to old ones
        centroidsJson = key.get_contents_as_string()
        newCentroids = json.loads(centroidsJson)
        
        kMeans = len(newCentroids)
        
        delta = 0.0
        for i in range(kMeans):
            delta += dist(newCentroids[i],oldCentroids[i])
        
        print delta

if __name__ == '__main__':
    main()