// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _XMLRPCREQUEST_H_
#define _XMLRPCREQUEST_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlHashMap.h>
#include "net/Url.h"
#include "net/HttpMessage.h"
#include "net/XmlRpcBody.h"
#include "net/XmlRpcResponse.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int XML_RPC_TIMEOUT = 20*1000;

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
 * This object is used to create a XML-RPC request to a specific remote XML-RPC
 * server. The caller is required to create this object for each XML-RPC request.
 * 
 * The caller uses the addParam(), addArray() and/or addStruct() functions to 
 * build up XML-RPC request content in XmlRpcBody.
 * 
 * - addParam() is for adding a param in the XmlRpcRequest.
 * 
 * All the param types must be UtlContainable. Here is the mapping between XML-RPC
 * types to UtlContainable types:
 * 
 * <i4> or <int> is UtlInt.
 * <boolean> is UtlBool.
 * <string> is UtlString.
 * <dateTime.iso8601> is UtlDateTime.
 * <array> is UtlSList.
 * <struct> is UtlHashMap.
 * 
 * <double> and <base64> are currently not supported.
 *  
 * The execute() function will close up the XML-RPC request frame and sends the
 * request to the remote server specified in the Url. The execute() function
 * receives the XmlRpcResponse from the remote server. If the return is true,
 * the caller can use getResponse() to obtain the response value. If the return
 * is false, the caller can use getFault() in XmlRpcResponse to get the fault
 * code and fault string.
 * 
 */

class XmlRpcRequest
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class XmlRpcTest;
   
/* ============================ CREATORS ================================== */

   /// Contruct a XML-RPC request for a given method
   XmlRpcRequest(Url& uri, ///< uri type can only be either http or https
                 const char* methodName ///< name of the method in XML-RPC request
                 );

   /// Destructor
   virtual ~XmlRpcRequest();

   /// Execute the named procedure on the remote server.
   bool execute(XmlRpcResponse& response); ///< response returned from the remote server
   /**<
    * @note
    * This is a synchronous (blocking) implementation (execute does not return
    * until it receives a response or an error).
    * 
    * If the return is false, the caller can use response.getFault() to obtain
    * the fault code and fault string.
    * 
    */

/* ============================ MANIPULATORS ============================== */

   /// Add an atomic param to the XML-RPC request
   bool addParam(UtlContainable* value); ///< value for the param
   
/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Add a value to the XML-RPC request
   bool addValue(UtlContainable* value);

   /// Add an array to the XML-RPC request
   bool addArray(UtlSList* array); ///< array of elements

   /// Add a struct to the XML-RPC request
   bool addStruct(UtlHashMap* members); ///< struct of members

   /// Url for the XML-RPC request
   Url mUrl;
   /**<
    * Only http or https can be used.
    */
   
   /// client for sending out XML-RPC request
   HttpMessage* mpHttpRequest;
   
   /// XML-RPC body
   XmlRpcBody* mpRequestBody;
    
   /// Disabled copy constructor
   XmlRpcRequest(const XmlRpcRequest& rXmlRpcRequest);

   /// Disabled assignment operator
   XmlRpcRequest& operator=(const XmlRpcRequest& rhs);   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCREQUEST_H_


