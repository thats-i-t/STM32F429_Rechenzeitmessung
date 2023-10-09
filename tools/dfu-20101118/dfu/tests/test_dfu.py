#!/usr/bin/python

import sys
sys.path.insert(0,'..')
import dfu

def test_seeed():
  print "PARSING Seeed LIB, APP and dumping images..."
  dfu.parse('seeed_lib_v2.5.dfu', dump_images=True)
  dfu.parse('seeed_app_v2.5e.dfu',dump_images=True)
  print "DONE."
  print "COMBINING Seeed LIB, APP in a single DFU..."
  targets = [
    [
      {
        'address': 0x08004000,
        'data':    open('seeed_lib_v2.5.dfu.target0.image0.bin','rb').read()
      },
      {
        'address': 0x0800792c,
        'data':    open('seeed_lib_v2.5.dfu.target0.image1.bin','rb').read()
      },
      {
        'address': 0x0800c000,
        'data':    open('seeed_app_v2.5e.dfu.target0.image0.bin','rb').read()
      },
    ],
  ]
  dfu.build('test_seeed.dfu',targets)
  print "DONE."
  print "PARSING generated DFU..."
  dfu.parse('test_seeed.dfu')
  print "DONE."

def test_benf():
  print "PARSING BenF LIB, APP and dumping images..."
  dfu.parse('benf_lib_v3.01.dfu', dump_images=True)
  dfu.parse('benf_app_v3.10.dfu', dump_images=True)
  print "DONE."
  print "COMBINING BenF LIB, APP in a single DFU..."
  targets = [
    [
      {
        'address': 0x08004000,
        'data':    open('benf_lib_v3.01.dfu.target0.image0.bin','rb').read()
      },
      {
        'address': 0x0800c000,
        'data':    open('benf_app_v3.10.dfu.target0.image0.bin','rb').read()
      },
    ],
  ]
  dfu.build('test_benf.dfu',targets)
  print "DONE."
  print "PARSING generated DFU..."
  dfu.parse('test_benf.dfu')
  print "DONE."

if __name__=="__main__":
  test_seeed()
  test_benf()
