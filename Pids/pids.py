# PID declarations  generated by parsepids.py  Aug 01, 2018  15:51:26

# PIDs (packet identifiers)
CONFIG=0x00	# for exchanging configurations on a link: spid + specifics; ie testframe
SPS_ACK=0x1	# confirm sps; link only - not networkable
SPS=0x2	# used for initializing SPS frame acks and setting id
PING=0x3	# to check other end
PING_BACK=0x4	# expected response to a PING
GET_VERSION=0x5	# get the version number
VERSION_NO=0x6	# return the version number
TALK_IN=0x7	# keyboard input to be interpreted
TALK_OUT=0x8	# used to send emits out usb port
EVAL=0x9	# evaluate text with destination; who, text
CALL_CODE=0xa	# call code starting at this location; who, addr32
MEMORY=0xb	# use for memory transactions: read/write/erase/confirm...
FILES=0xc	# user for file transactions: read/write/delete/...
MAX_PIDS=0xd	# number of PIDS defined

pids = {
	CONFIG:"CONFIG",	# for exchanging configurations on a link: spid + specifics; ie testframe
	SPS_ACK:"SPS_ACK",	# confirm sps; link only - not networkable
	SPS:"SPS",	# used for initializing SPS frame acks and setting id
	PING:"PING",	# to check other end
	PING_BACK:"PING_BACK",	# expected response to a PING
	GET_VERSION:"GET_VERSION",	# get the version number
	VERSION_NO:"VERSION_NO",	# return the version number
	TALK_IN:"TALK_IN",	# keyboard input to be interpreted
	TALK_OUT:"TALK_OUT",	# used to send emits out usb port
	EVAL:"EVAL",	# evaluate text with destination; who, text
	CALL_CODE:"CALL_CODE",	# call code starting at this location; who, addr32
	MEMORY:"MEMORY",	# use for memory transactions: read/write/erase/confirm...
	FILES:"FILES",	# user for file transactions: read/write/delete/...
	MAX_PIDS:"MAX_PIDS"	# number of PIDS defined
}

# macros
ACK_BIT=0x80	# used for indicating SPS frames
SPS_BIT=0x40	# used for indicating type of an SPS frame
PID_BITS=(0xFF&(~(ACK_BIT|SPS_BIT)))	# used to mask off upper bits
WHO_PIDS=(SPS_ACK)	# all pids greater than, use the who header for routing
MAX_FRAME_LENGTH=254	# no more than 254; set according to link resources
MAX_FRAMES=30	# number of frames floating around system
SPS_RETRIES=5	# maximum number of SPS retries
SFP_POLL_TIME=2	# polling in link down
SFP_SPS_TIME=2500	# time between retransmissions
SFP_FRAME_TIME=50	# maximum time to wait between bytes for a frame
SFP_FRAME_PROCESS=1000	# maximum time to wait for frame processing
STALE_RX_FRAME=1000	# number of milliseconds to hang onto a received frame
NUM_LINKS=1	# number of links per node

# whos
DIRECT=0	# 
HOST=(DIRECT)	# Aliases
ME=(DIRECT)	# 
YOU=(DIRECT)	# 
MAIN_CPU=0x1	# target cpu main port
MAIN_HOST=0x2	# host ports on cpus
ROUTING_POINTS=0x3	# number of points for routing

# who dictionary
whoDict = {
    'Direct': DIRECT,
    'Host': HOST,
    'Me': ME,
    'You': YOU,
    'Main Cpu': MAIN_CPU,
    'Main Host': MAIN_HOST,
    'Routing Points': ROUTING_POINTS,
	'':0
}