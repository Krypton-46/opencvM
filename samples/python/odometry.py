#!/usr/bin/env python

import numpy as np
import cv2 as cv

import argparse
import sys

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--algo',
        help="""DEPTH - works with depth,
                RGB - works with images,
                RGB_DEPTH - works with all,
                defalt - runs all algos""",
        default="")
    parser.add_argument(
        '-src_d',
        '--source_depth_frame',
        default="")
    parser.add_argument(
        '-dst_d',
        '--destination_depth_frame',
        default="")
    parser.add_argument(
        '-src_rgb',
        '--source_rgb_frame',
        default="")
    parser.add_argument(
        '-dst_rgb',
        '--destination_rgb_frame',
        default="")

    args = parser.parse_args()

    depth1 = cv.imread(args.source_depth_frame, cv.IMREAD_ANYDEPTH).astype(np.float32)
    depth2 = cv.imread(args.destination_depth_frame, cv.IMREAD_ANYDEPTH).astype(np.float32)

    rgb1 = cv.imread(args.source_rgb_frame, cv.IMREAD_COLOR)
    rgb2 = cv.imread(args.destination_rgb_frame, cv.IMREAD_COLOR)

    if args.algo == "DEPTH" or args.algo == "":
        odometry = cv.Odometry(cv.DEPTH)
        Rt = np.zeros((4, 4))
        odometry.compute(depth1, depth2, Rt)
        print(Rt)
    if args.algo == "RGB" or args.algo == "":
        odometry = cv.Odometry(cv.RGB)
        Rt = np.zeros((4, 4))
        odometry.compute(rgb1, rgb2, Rt)
        print(Rt)
    if args.algo == "RGB_DEPTH" or args.algo == "":
        odometry = cv.Odometry(cv.RGB_DEPTH)
        Rt = np.zeros((4, 4))
        odometry.compute(depth1, rgb1, depth2, rgb2, Rt)
        print(Rt)



if __name__ == '__main__':
    main()
