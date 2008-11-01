//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprFromNet_h_
#define _MprFromNet_h_

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#ifdef _WIN32 /* [ */
#include <winsock2.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSocket.h"
#include "mp/NetInTask.h"
#include "mp/MpUdpBuf.h"
#include "mp/MpRtpBuf.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/IRTPDispatch.h"
#include "rtcp/INetDispatch.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprDecode;
class MprDejitter;
class MprRtpDispatcher;

/// The "From Network" media processing resource
class MprFromNet
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprFromNet();

     /// Destructor
   virtual ~MprFromNet();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @brief Set the inbound RTP and RTCP sockets.
   OsStatus setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     /**< @returns Always OS_SUCCESS for now. */

     /// @brief Unregister the inbound RTP and RTCP sockets.
   OsStatus resetSockets();
     /**< @returns Always OS_SUCCESS for now. */

     /// Take in a buffer from the NetIn task
   OsStatus pushPacket(const MpUdpBufPtr &buf, bool isRtcp);

     /// Enable/disable discarding of given RTP stream.
   OsStatus enableSsrcDiscard(UtlBoolean enable, RtpSRC ssrc);
     /**<
     *  @param[in] enable - should given stream be discarded or not.
     *  @param[in] ssrc - SSRC of the stream to discard. If \p enable is
     *             FALSE, then \p ssrc is ignored.
     *
     *  @note Only one stream at a time can be discarded. This functionality
     *        is designed to discard looped local back packets.
     *
     *  @returns Always OS_SUCCESS for now.
     */

     /// Set RTP dispatcher instance
   OsStatus setRtpDispatcher(MprRtpDispatcher *pRtpDispatcher);
     /**<
     *  @note Must be called right after object construction!
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

#ifdef INCLUDE_RTCP /* [ */
     /// @brief These accessors were added by DMG to allow a Connection to access and modify
     /// RTP and RTCP stream informations
   void setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch);

#else /* INCLUDE_RTCP ] [ */
     /// retrieve the RR info needed to complete an RTCP packet
   OsStatus getRtcpStats(MprRtcpStats& stats);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex          mMutex;
   NetInTask*       mNetInTask;
   UtlBoolean       mRegistered;
   MprRtpDispatcher* mpRtpDispatcher;
   UtlBoolean       mDiscardSelectedStream;
   RtpSRC           mDiscardedSSRC;
#ifdef INCLUDE_RTCP /* [ */
   INetDispatch*    mpiRTCPDispatch;
   IRTPDispatch*    mpiRTPDispatch;
#else /* INCLUDE_RTCP ] [ */
   rtpHandle        mInRtpHandle;
   int              mRtcpCount;
#endif /* INCLUDE_RTCP ] */

   int             mNumPushed;     ///< Total RTP+RTCP pkts received from NetIn
   int             mNumPktsRtcp;   ///< Total RTCP packets received from NetIn
   int             mNumPktsRtp;    ///< Total RTP packets received from NetIn
   int             mNumEncDropped; ///< Encoded RTP packets dropped due to no key
   int             mNumLoopDropped;///< Looped-back mcast RTP packets dropped

#ifndef INCLUDE_RTCP /* [ */
     /// Update the RR info for the current incoming packet
   OsStatus rtcpStats(struct RtpHeader *h);
#endif /* INCLUDE_RTCP ] */

     /// Parse UDP packet and return filled RTP packet buffer.
   static MpRtpBufPtr parseRtpPacket(const MpUdpBufPtr &buf);

     /// Copy constructor (not implemented for this class)
   MprFromNet(const MprFromNet& rMprFromNet);

     /// Assignment operator (not implemented for this class)
   MprFromNet& operator=(const MprFromNet& rhs);

};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
inline  void  MprFromNet::setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch)
{
// Set the dispatch pointers for both RTP and RTCP
   mpiRTPDispatch   = piRTPDispatch;
   mpiRTCPDispatch  = piRTCPDispatch;
}

#endif /* INCLUDE_RTCP ] */

#endif  // _MprFromNet_h_
