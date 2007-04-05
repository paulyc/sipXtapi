//  
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprToneGenConstructor_h_
#define _MprToneGenConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MprToneGen.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprToneGenConstructor is used to contruct a ToneGen resource
*
*/
class MprToneGenConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MprToneGenConstructor(int samplesPerFrame = 80, 
                          int samplesPerSecond = 8000,
                          const UtlString& locale = "") :
      MpAudioResourceConstructor(DEFAULT_TONE_GEN_RESOURCE_TYPE,
                                 0, //minInputs,
                                 1, //maxInputs,
                                 1, //minOutputs,
                                 1, //maxOutputs,
                                 samplesPerFrame,
                                 samplesPerSecond),
      mLocale(locale)
    {
    };

    /** Destructor
     */
    virtual ~MprToneGenConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual MpResource* newResource(const UtlString& resourceName)
    {
        assert(mSamplesPerFrame > 0);
        assert(mSamplesPerSecond > 0);

        // TODO: use MprToneGen instead
        return(new MprToneGen(resourceName,
                             mSamplesPerFrame,
                             mSamplesPerSecond,
                             mLocale));
    }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mLocale;

    /** Disabled copy constructor
     */
    MprToneGenConstructor(const MprToneGenConstructor& rMprToneGenConstructor);


    /** Disable assignment operator
     */
    MprToneGenConstructor& operator=(const MprToneGenConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToneGenConstructor_h_
