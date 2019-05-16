/*
 * @File: ieee1905_defs.h
 *
 * @Abstract: IEEE 1905.1 definition header file.
 *
 * @Notes:
 *
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All rights reserved.
 *
 */

#ifndef ieee1905_defs__h /*once only*/
#define ieee1905_defs__h

#include <sys/types.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#undef IEEE1905_USE_BCAST

/*
 * Packet header macros
 */
#define IEEE1905_ETHER_TYPE         (0x893A) /* IEEE 1905.1 Ethertype*/
#ifdef IEEE1905_USE_BCAST
#define IEEE1905_MULTICAST_ADDR     "\xFF\xFF\xFF\xFF\xFF\xFF" /* DEBUG ONLY! */
#else
#define IEEE1905_MULTICAST_ADDR     "\x01\x80\xC2\x00\x00\x13" /* IEEE 1905.1 Multicast address */
#endif

#define IEEE1905_OUI_LENGTH     3

#define IEEE1905_ETH_HEAD_LEN       (sizeof(struct ether_header))
#define IEEE1905_HEAD_LEN           (sizeof(struct ieee1905Header_t))
#define IEEE1905_TLV_MIN_LEN        (sizeof(u_int8_t) + sizeof(u_int16_t))
#define IEEE1905_TLV_LEN( _len )    (sizeof(u_int8_t) + sizeof(u_int16_t) + _len)
#define IEEE1905_FRAME_MIN_LEN      (IEEE1905_ETH_HEAD_LEN + IEEE1905_HEAD_LEN + IEEE1905_TLV_MIN_LEN)
#define IEEE1905_CONTENT_MAXLEN     (ETH_FRAME_LEN - IEEE1905_HEAD_LEN - IEEE1905_ETH_HEAD_LEN)

/*
 * Supported IEEE 1905.1 protocol version
 */
#define IEEE1905_PROTOCOL_VERSION  0x00

/*
 * IEEE 1905.1 header flags
 */
#define IEEE1905_HEADER_FLAG_LAST_FRAGMENT      ( 1 << 7 )  /* Last fragment */
#define IEEE1905_HEADER_FLAG_RELAY              ( 1 << 6 )  /* Relay message */

#define ieee1905IsMessageFragmented( _flags ) (!( _flags & IEEE1905_HEADER_FLAG_LAST_FRAGMENT ))

/*
 * IEEE 1905.1 Topology Discovery message timeout
 */
#define IEEE1905_TOPOLOGY_DISCOVERY_TIMEOUT     ( 60 ) /* Seconds */

/*
 * IEEE 1905.1 message types
 */
typedef enum ieee1905MessageType_e
{
    IEEE1905_MSG_TYPE_TOPOLOGY_DISCOVERY = 0,
    IEEE1905_MSG_TYPE_TOPOLOGY_NOTIFICATION,
    IEEE1905_MSG_TYPE_TOPOLOGY_QUERY,
    IEEE1905_MSG_TYPE_TOPOLOGY_RESPONSE,
    IEEE1905_MSG_TYPE_VENDOR_SPECIFIC,
    IEEE1905_MSG_TYPE_LINK_METRIC_QUERY,
    IEEE1905_MSG_TYPE_LINK_METRIC_RESPONSE,
    IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_SEARCH,
    IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_RESPONSE,
    IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_WPS,
    IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_RENEW,
    IEEE1905_MSG_TYPE_PB_EVENT_NOTIFICATION,
    IEEE1905_MSG_TYPE_PB_JOIN_NOTIFICATION,

    IEEE1905_MSG_TYPE_RESERVED /* Must be the last */

} ieee1905MessageType_e;

/*
 * IEEE 1905.1 control frame header
 */
typedef struct ieee1905Header_t
{
    u_int8_t    version;    /* Version of IEEE 1905.1 protocol used in frame */
    u_int8_t    reserved;   /* Reserved */
    u_int16_t   type;       /* Message type */
    u_int16_t   mid;        /* Message identifier */
    u_int8_t    fid;        /* Fragment identifier */
    u_int8_t    flags;      /* Flags */

} ieee1905Header_t;

/*
 * IEEE 1905.1 TLV
 */
typedef enum ieee1905TlvType_e
{
    IEEE1905_TLV_TYPE_END_OF_MESSAGE = 0,
    IEEE1905_TLV_TYPE_AL_ID = 1,
    IEEE1905_TLV_TYPE_MAC_ID = 2,
    IEEE1905_TLV_TYPE_DEVICE_INFORMATION = 3,
    IEEE1905_TLV_TYPE_DEVICE_BRIDGING_CAPABILITY = 4,
    IEEE1905_TLV_TYPE_MEDIA_TYPE = 5,
    IEEE1905_TLV_TYPE_LEGACY_NEIGHBOR = 6,
    IEEE1905_TLV_TYPE_NEIGHBOR_DEVICE = 7,
    IEEE1905_TLV_TYPE_LINK_METRIC_QUERY = 8,
    IEEE1905_TLV_TYPE_TRANSMITTER_LINK_METRIC_RESPONSE = 9,
    IEEE1905_TLV_TYPE_RECEIVER_LINK_METRIC_RESPONSE = 10,
    IEEE1905_TLV_TYPE_VENDOR_SPECIFIC = 11,
    IEEE1905_TLV_TYPE_RESULT_CODE = 12,
    IEEE1905_TLV_TYPE_SEARCHED_ROLE = 13,
    IEEE1905_TLV_TYPE_FREQ_BAND = 14,
    IEEE1905_TLV_TYPE_SUPPORTED_ROLE = 15,
    IEEE1905_TLV_TYPE_SUPPORTED_FREQ_BAND = 16,
    IEEE1905_TLV_TYPE_WPS = 17,
    IEEE1905_TLV_TYPE_PUSH_BUTTON_EVENT = 18,
    IEEE1905_TLV_TYPE_PUSH_BUTTON_JOIN = 19,

    IEEE1905_TLV_TYPE_RESERVED /* Must be the last */

} ieee1905TlvType_e;

/*
 * IEEE1905.1 Media types
 */

typedef enum
{
    IEEE1905_MEDIA_TYPE_IEEE802_3,
    IEEE1905_MEDIA_TYPE_IEEE802_11,
    IEEE1905_MEDIA_TYPE_IEEE1901,
    IEEE1905_MEDIA_TYPE_MOCA,

    IEEE1905_MEDIA_TYPE_RESERVED,
    IEEE1905_MEDIA_TYPE_UNKNOWN = 255

} ieee1905MediaType_e;

enum
{
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_3U_FAST_ETHERNET,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_3AB_GIGABIT_ETHERNET,
};

enum
{
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11B_2_4G,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11G_2_4G,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11A_5G,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11N_2_4G,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11N_5G,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11AC_5G,
    IEEE1905_MEDIA_DESCRIPTION_IEEE802_11AD_60G,

    IEEE1905_MEDIA_DESCRIPTION_IEEE802_RESERVED

};

enum
{
    IEEE1905_MEDIA_DESCRIPTION_IEEE1901_WAVELET,
    IEEE1905_MEDIA_DESCRIPTION_IEEE1901_OFDM,
};

enum
{
    IEEE1905_MEDIA_DESCRIPTION_MOCA_V1_1,
};

#define IEEE1905_SPECIFIC_INFO_IEEE80211( _info ) ( _info << 6 )
#define IEEE1905_SPECIFIC_INFO_IEEE80211_EXTRACT_ROLE( _info ) ( _info >> 6 )

enum
{
    IEEE1905_SPECIFIC_INFO_IEEE80211_AP,
    IEEE1905_SPECIFIC_INFO_IEEE80211_STATION,
    IEEE1905_SPECIFIC_INFO_IEEE80211_P2P,

    IEEE1905_SPECIFIC_INFO_IEEE80211_RESERVED
};

typedef struct ieee1905MediaType_t {
    u_int8_t medtypeClass;
    u_int8_t medtypePhy;
    u_int8_t val_length;
    u_int8_t val[ 0 ];
} ieee1905MediaType_t;

typedef struct ieee1905MediaSpecificHPAV_t
{
    u_int8_t avln[7];
} ieee1905MediaSpecificHPAV_t;

typedef struct ieee1905MediaSpecificWiFi_t
{
    u_int8_t bssid[ETH_ALEN];
    u_int8_t role;
    u_int8_t reserved[3];
} ieee1905MediaSpecificWiFi_t;

/*
 * Legacy bridges
 */
enum
{
    IEEE1905_LEGACY_BRIDGES_NONE,
    IEEE1905_LEGACY_BRIDGES_EXIST,
};


/*
 * IEEE 1905.1 control frame content(one or more TLV).
 *          -----------
 *         |    TLV    |
 *          -----------
 *         |    TLV    |
 *          -----------
 *         |    ...    |
 *          -----------
 *         | EndOfMsg  |
 *          -----------
 */
typedef struct ieee1905TLV_t
{
    u_int8_t type;      /* Type of TLV */
    u_int16_t length;   /* Length of contents */
    u_int8_t val[ 0 ];  /* Contents data */

} __attribute__((packed)) ieee1905TLV_t;

/*
 * Complete Ethernet IEEE 1905.1 control frame message
 */
typedef struct ieee1905Message_t
{
    struct ether_header etherHeader;
    struct ieee1905Header_t ieee1905Header;

    u_int8_t content[ IEEE1905_CONTENT_MAXLEN ];

} ieee1905Message_t;

/*
 * Generic TLV structures used in multiple message types
 */

typedef struct ieee1905NeighbourDevice_t
{
    struct ether_addr addr;
    u_int8_t legacyBridge;

} __attribute__((packed)) ieee1905NeighbourDevice_t;


typedef struct ieee1905SingleAddressTLV_t
{
    ieee1905TLV_t tlvHeader;
    struct ether_addr mac;
} ieee1905SingleAddressTLV_t;

typedef struct ieee1905VendorSpecificHeaderTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t oui[ IEEE1905_OUI_LENGTH ];
    u_int8_t val[ 0 ];
} ieee1905VendorSpecificHeaderTLV_t;

/*
 * Link metrics structures and enumerations
 */

enum /* used in queryScope field */
{
    IEEE1905_LINK_METRIC_SCOPE_ALL_NEIGHBORS = 0,
    IEEE1905_LINK_METRIC_SCOPE_SPECIFIC_NEIGHBOR,

    IEEE1905_LINK_METRIC_SCOPE_RESERVED
};

enum /* used in requestedMetrics field */
{
    IEEE1905_LINK_METRIC_REQ_TX = 0,
    IEEE1905_LINK_METRIC_REQ_RX,
    IEEE1905_LINK_METRIC_REQ_TX_RX,

    IEEE1905_LINK_METRIC_REQ_RESERVED
};

enum
{
    IEEE1905_LINK_METRIC_RESPONSE_INVALID_NEIGHBOR = 0
};

typedef struct ieee1905LinkMetricQuery1TLV_t /* used with queryScope == IEEE1905_LINK_METRIC_SCOPE_SPECIFIC_NEIGHBOR */
{
    ieee1905TLV_t tlvHeader;
    u_int8_t queryScope;
    struct ether_addr neighborAlId;
    u_int8_t requestedMetrics;
} __attribute__((packed)) ieee1905LinkMetricQuery1TLV_t;

typedef struct ieee1905LinkMetricQuery2TLV_t /* used with queryScope == IEEE1905_LINK_METRIC_SCOPE_ALL_NEIGHBORS */
{
    ieee1905TLV_t tlvHeader;
    u_int8_t queryScope;
    u_int8_t requestedMetrics;
} __attribute__((packed)) ieee1905LinkMetricQuery2TLV_t;

/*
 * AP Auto-Configuration structures and enumerations
 */

enum /* used in searchedRole and supportedRole fields */
{
    IEEE1905_AP_AUTOCONFIG_ROLE_REGISTRAR = 0,
    IEEE1905_AP_AUTOCONFIG_ROLE_AP_ENROLLEE
};

enum /* used in freqBand and supportedFreqBand fields */
{
    IEEE1905_AP_AUTOCONFIG_FREQ_BAND_2P4G = 0,
    IEEE1905_AP_AUTOCONFIG_FREQ_BAND_5G,
    IEEE1905_AP_AUTOCONFIG_FREQ_BAND_60G
};

typedef struct ieee1905APAutoConfigSearchedRoleTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t searchedRole;
} ieee1905APAutoConfigSearchedRoleTLV_t;

typedef struct ieee1905APAutoConfigFreqBandTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t freqBand;
} ieee1905APAutoConfigFreqBandTLV_t;

typedef struct ieee1905APAutoConfigSupportedRoleTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t supportedRole;
} ieee1905APAutoConfigSupportedRoleTLV_t;

typedef struct ieee1905APAutoConfigSupportedFreqBandTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t supportedFreqBand;
} ieee1905APAutoConfigSupportedFreqBandTLV_t;

typedef struct ieee1905APAutoConfigWPSTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t wps[0];
} ieee1905APAutoConfigWPSTLV_t;

/*
 * Push Button structures
 */

typedef struct ieee1905PushButtonEventTLV_t
{
    ieee1905TLV_t tlvHeader;
    u_int8_t numEntries;
    u_int8_t val[ 0 ];
} ieee1905PushButtonEventTLV_t;

typedef struct ieee1905PushButtonJoinTLV_t
{
    ieee1905TLV_t tlvHeader;
    struct ether_addr alID;
    u_int16_t midPBEvent;
    struct ether_addr txIfMac;
    struct ether_addr newIfMac;
} __attribute__((packed)) ieee1905PushButtonJoinTLV_t;

/*-------------------------------------------------------------------*/
/*---------------------------IEEE1905 API----------------------------*/
/*-------------------------------------------------------------------*/

/*
 * TLV Handling API
 */
#define ieee1905TLVTypeGet( _TLV ) \
    ( (_TLV)->type )

#define ieee1905TLVTypeSet( _TLV, _type ) \
    do{ (_TLV)->type = _type; (_TLV)->length = 0;} while(0)

#define ieee1905TLVLenGet( _TLV ) \
    ( htons( (_TLV)->length ) )

#define ieee1905TLVLenSet( _TLV, _length, _total ) \
    do{ (_TLV)->length = htons( _length ); ( _total ) += ( _length ) + IEEE1905_TLV_MIN_LEN; } while(0)

#define ieee1905TLVValGet( _TLV ) \
    ( (_TLV)->val )

#define ieee1905EndOfTLVSet( _TLV ) \
    do{ (_TLV)->type = IEEE1905_TLV_TYPE_END_OF_MESSAGE; (_TLV)->length = 0; } while(0)

#define ieee1905TLVGetNext( _TLV ) \
    ((ieee1905TLV_t *)((u_int8_t *)(_TLV) + htons( (_TLV)->length ) + IEEE1905_TLV_MIN_LEN))

#define ieee1905TLVValSet( _TLV, _val, _len ) \
    do{ (_TLV)->length = htons(_len) ; memcpy((_TLV)->val, (_val), (_len) ); } while(0)

#define ieee1905TLVSet( _TLV, _type, _len, _val, _total ) \
    do{ (_TLV)->type = _type; (_TLV)->length = htons(_len); memcpy((_TLV)->val, (_val), (_len) ); ( _total ) += ( _len ) + IEEE1905_TLV_MIN_LEN; } while(0)

#endif /* ieee1905_defs__h */
