import numpy as np
import time, re

from .. import ROOT
from .. import larcv

from iomanager import IOManager
from imagefactory import ImageFactory

# data manger helps get the producers from the ROOT file
# as well as manage factory creation of images as the user
# hits replot, next/prev event

class DataManager(object):

    def __init__(self,argv):
        
        self.iom = IOManager(argv)
        self.keys ={}

        self.IF = ImageFactory()
        
        # get keys from rootfile, iterate over the enum
        # and see what's in the root file
        for i in xrange(larcv.kProductUnknown):
            product = larcv.ProductName(i)

            self.keys[product] = []

            producers=self.iom.iom.producer_list(i)
            
            for p in producers:
                self.keys[product].append(p)

        # run subrun and event start at zero
        self.run    = -1
        self.subrun = -1
        self.event  = -1
        

    def get_nchannels(self,ii,imgprod) :
        # Sorry Vic I hacked this
        # --> it's ok
        self.iom.read_entry(ii)
        imdata  = self.iom.get_data(larcv.kProductImage2D,imgprod)
        return imdata.Image2DArray().size()

    def get_event_image(self,ii,imgprod,roiprod,planes, refresh=True) :

        #Load data in TChain
        self.iom.read_entry(ii)

        # there may be no ROI
        hasroi = False
        roidata = None
        if roiprod is not None:
            roidata = self.iom.iom.get_data(larcv.kProductROI,roiprod)
            roidata = roidata.ROIArray()
            hasroi = True

        # get the EventImage2D
        imdata  = self.iom.get_data(larcv.kProductImage2D,imgprod) # goes to disk

        self.run    = imdata.run()
        self.subrun = imdata.subrun()
        self.event  = imdata.event()

        # get the std::vector<larcv::Image2D>
        imdata  = imdata.Image2DArray()
        if imdata.size() == 0 : return (None, False)

        # hand it off to the factory, the producer name should query the
        # the correct subclass of PlotImage
        image = self.IF.get(imdata,roidata,planes,imgprod) # returns PlotImgae

        # return it to rgbviewer
        return ( image, hasroi )         
    
    
    
    
    # -----------------------------------------------------------------------------
    # Erez, July-21, 2016 - get an image using R/S/E navigation
    # -----------------------------------------------------------------------------
    def get_rse_image(self,wanted_rse,imgprod,roiprod,planes, refresh=True) :
        

        for entry in range(self.iom.get_n_entries()):
            #            print entry
            read_entry = self.iom.read_entry(entry)
            event_base = self.iom.get_data(larcv.kProductImage2D,imgprod)
            curren_rse = [event_base.run(),event_base.subrun(),event_base.event()]

            if curren_rse == wanted_rse:
                ii = entry
                break

        return self.get_event_image(ii,imgprod,roiprod,planes,refresh)
    # -----------------------------------------------------------------------------










