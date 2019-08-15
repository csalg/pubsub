//#ifndef PUBSUB_PARAMS_H
//#define PUBSUB_PARAMS_H
//
//extern const unsigned short MAX_SUBS;
//extern const unsigned short MAX_ATTS;
//extern const unsigned short MAX_BUCKS;
//extern const unsigned short SEGMENTS;
//extern const unsigned short MAX_SIGNATURE;
//extern unsigned short MAX_SPAN;
//extern unsigned MAX_CARDINALITY;
//extern unsigned short SPAN_AFTER_CLUSTERING;
//extern unsigned short K;
//extern unsigned short LEAVE_OUT;
//
//
////
//// Created by Work on 2019-08-14.
////

#ifndef PUBSUB_PARAMS_H
#define PUBSUB_PARAMS_H

static const unsigned       MAX_SUBS = 100000000;
static const unsigned short MAX_ATTS = 10;

static const unsigned short MAX_BUCKS = 500;

static const unsigned short SEGMENTS = 16;
static const unsigned short MAX_SIGNATURE = 61;

static unsigned short       MAX_SPAN = 5000;
static unsigned             MAX_CARDINALITY = 1000000000;
static unsigned short       SPAN_AFTER_CLUSTERING= 2500;
static unsigned short       K= 100;
static unsigned short       LEAVE_OUT= 0;

#endif //PUBSUB_PARAMS_H
