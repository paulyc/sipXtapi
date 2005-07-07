/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.phone;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import org.apache.regexp.RE;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import java.rmi.RemoteException;
import java.util.Collection;


/**
 * DeviceBean is the EJB implementation of Device.
 *
 * @author ibutcher
 */

public class DeviceBean extends JDBCAwareEJB implements EntityBean, DeviceBusiness {


//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private EntityContext mCTX;

    /**
     *  Home Interfaces
     */
    private OrganizationHome mOrganizationHome;
    private DeviceTypeHome mDeviceTypeHome;
    private ManufacturerHome mManufacturerHome;


    // Clearing Entity Bean references
    private Organization mOrganizationEJBObject;
    private DeviceType mDeviceTypeEJBObject;
    private Manufacturer mManufacturerEJBObject;

    // misc.
    private RE SERIAL_NUMBER_RE;

    // Entity EJ Bean attributes

    /**
     * id is the PK for the Device.  It is auto-generated by a database
     * sequence.
     */
    public Integer id;

    /**
     * userID is the PK of a User who 'own's this Device.
     */
    public String userID;

    /**
     * deviceGroupID is the PK of the DeviceGroup that this Device is
     * part of/assigned to.
     */
    public Integer deviceGroupID;

    /**
     * refConfigSetID is the PK of the RefConfigurationSet that is assigned
     * to this Device.
     */
    public Integer refConfigSetID;

    /**
     * coreSoftwareDetailsID is the PK of the CoreSoftwareDetails that
     * this Device is thought to be running.
     */
    public Integer coreSoftwareDetailsID;

    /**
     * a free-form description of the device.  This might describe it's
     * function or location.
     */
    public String description;

    /**
     * a short identity by which the device is uniquely known.  This is
     * the 'ID' by which the Device is known in the UI.
     */
    public String shortName;

    /**
     * organizationID is the PK of the Organization that this Device
     * belongs to.
     */
    public Integer organizationID;

    /**
     * PK of the DeviceType that this Device is an instance of.
     */
    public Integer deviceTypeID;

    /**
     * serialNumber is the uniquely identifying value of the physical device/
     * handset that this Device represents.  For Pingetl and Cisco _phones_
     * this is their MAC address.   Over time a Device may have many different
     * serialNumbers and physical phones are replaced.  This separation makes
     * the subsitituion of broken phones possible whilst maintaining all other
     * settings for the Device.
     */
    public String serialNumber;



//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Standard  Entity Bean Method Implementation.
     *
     * @param shortName the display name by which the Device will be known
     * (shown in the user interfaces).
     * @param organizationID PK of the Organization that this Device belongs
     * to.
     * @param coreSoftwareDetailsID PK of the CoreSoftwareDetails that this
     * Device runs.  Not very interesting at the moment.
     * @param refConfigSetID PK of the RefConfigurationSet that this Device
     * is to have assigned to it.  This will ultimately define which
     * properties will end up this Device's profiles.
     * @param description short description of the Device, where is lives,
     * etc.
     * @param deviceGroupID PK of the DeviceGroup that this Device belongs
     * to.
     * @param userID PK of the User who owns this Device.
     * @param deviceTypeID PK of the DeviceType that is an instance of.
     * @param serialNumber the uniquely identifiying externally viewable
     * identity of this Device.  In the case of Pingtel and Cisco Devices
     * this is their MAC address.
     * @return PK of the new Device
     * @throws CreateException is thrown for non-fatal containter
     * generated errors (standard EJB pattern).
     * @throws PDSException is thrown for application level errors.
     */
    public Integer ejbCreate(   String shortName,
                                Integer organizationID,
                                Integer coreSoftwareDetailsID,
                                Integer refConfigSetID,
                                String description,
                                Integer deviceGroupID,
                                String userID,
                                Integer deviceTypeID,
                                String serialNumber )

        throws CreateException, PDSException {

        setId(new Integer(this.getNextSequenceValue("LOG_PHN_SEQ")));
        setShortName(shortName);
        setOrganizationID(organizationID);
        setCoreSoftwareDetailsID(coreSoftwareDetailsID);
        setRefConfigSetID(refConfigSetID);
        setDescription(description);
        setDeviceGroupID(deviceGroupID);
        setUserID(userID);
        setDeviceTypeID(deviceTypeID);
        setSerialNumber(serialNumber);

        return null;
    }


    /**
     * Standard  Entity Bean Method Implementation.
     *
     * @param shortName the display name by which the Device will be known
     * (shown in the user interfaces).
     * @param organizationID PK of the Organization that this Device belongs
     * to.
     * @param coreSoftwareDetailsID PK of the CoreSoftwareDetails that this
     * Device runs.  Not very interesting at the moment.
     * @param refConfigSetID PK of the RefConfigurationSet that this Device
     * is to have assigned to it.  This will ultimately define which
     * properties will end up this Device's profiles.
     * @param description short description of the Device, where is lives,
     * etc.
     * @param deviceGroupID PK of the DeviceGroup that this Device belongs
     * to.
     * @param userID PK of the User who owns this Device.
     * @param deviceTypeID PK of the DeviceType that is an instance of.
     * @param serialNumber the uniquely identifiying externally viewable
     * identity of this Device.  In the case of Pingtel and Cisco Devices
     * this is their MAC address.
     */
    public void ejbPostCreate(  String shortName,
                                Integer organizationID,
                                Integer coreSoftwareDetailsID,
                                Integer refConfigSetID,
                                String description,
                                Integer deviceGroupID,
                                String userID,
                                Integer deviceTypeID,
                                String serialNumber ) { }


    /**
     *  Standard  Entity Bean Method Implementation
     */
    public void ejbLoad() { }


    /**
     *  Standard  Entity Bean Method Implementation
     */
    public void ejbStore() { }


    /**
     *  Standard  Entity Bean Method Implementation
     */
    public void ejbRemove() { }


    /**
     *  Standard  Entity Bean Method Implementation
     */
    public void ejbActivate() {
        mOrganizationEJBObject = null;
        mDeviceTypeEJBObject = null;
        mManufacturerEJBObject = null;
     }


    public void ejbPassivate() { }


    /**
     * Standard Entity Bean Method Implementation.
     *
     *@param  ctx  The new entityContext value
     */
    public void setEntityContext(EntityContext ctx) {
        this.mCTX = ctx;

        try {
            Context initial = new InitialContext();

            mDeviceTypeHome = (DeviceTypeHome) initial.lookup("DeviceType");
            mManufacturerHome = (ManufacturerHome) initial.lookup("Manufacturer");
            mOrganizationHome = (OrganizationHome) initial.lookup ("Organization" );

            SERIAL_NUMBER_RE = new RE("^[0-9a-fA-F]{12}$");
        }
        catch (Exception ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }
    }


    /**
     *  Boiler Plage Entity Bean Implementation
     */
    public void unsetEntityContext() {
        this.mCTX = null;
    }


    /**
     *@return  the value of Devices' id (it's PK)
     */
    public Integer getID() {
        return this.id;
    }


    /**
     *  Gets the organizationID attribute of the LogicalPhoneBean object
     *
     *@return    The organizationID value
     */
    public Integer getOrganizationID() {
        return this.organizationID;
    }


    /**
     * gets the shortName attribute of the Device
     *
     * @return shortName of the Device
     */
    public String getShortName() {
        return this.shortName;
    }


    /**
     *  sets the shortName attribute of the LogicalPhoneBean object
     *
     *@param  shortName  The new shortName value
     */
    public void setShortName(String shortName) throws PDSException {

        // device shortNames can not be null
        if (shortName == null){
            mCTX.setRollbackOnly();
            throw new PDSException(collateErrorMessages("E3036", null));
        }

        // nor can they be too long to fit in the database.
        if (shortName != null && shortName.length() > MAX_SHORT_NAME_LEN ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3028",
                                        new Object[]{
                                            "name",
                                            new Integer(MAX_SHORT_NAME_LEN)}));
        }

        // see if another Device is already using this serial number
        Collection existing = null;
        try {
            existing = ((DeviceHome)mCTX.getEJBHome()).findByShortName( shortName );
        }
        catch (Exception e) {
            logFatal (e.getMessage(), e);
            throw new EJBException (e.getMessage());
        }

        if (!existing.isEmpty()) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E2034", new Object [] {shortName}));
        }

        this.shortName = shortName;
    }


    /**
     * returns the userID of this Device.
     *
     * @return the value of the User who own's this Device
     */
    public String getUserID() {
        return this.userID;
    }


    /**
     * sets the userID of this Device.
     *
     * @param userID the primary key of the User who is to own this Device
     */
    public void setUserID(String userID) {
        this.userID = userID;
    }


    /**
     * gets the DeviceGroupID of this Device.
     *
     * @return the DeviceGroupID which this Device belongs to
     */
    public Integer getDeviceGroupID() {
        return this.deviceGroupID;
    }


    /**
     * sets the DeviceGroupID for this Device.
     *
     * @param  deviceGroupID  the PK of the DeviceGroup which you wish to
     * add this Device to.
     */
    public void setDeviceGroupID(Integer deviceGroupID) throws PDSException {
        // devices must belong to a group.
        if (deviceGroupID == null){
            mCTX.setRollbackOnly();
            throw new PDSException(collateErrorMessages("E3037", null));
        }
        this.deviceGroupID = deviceGroupID;
    }



    /**
     * gets the refConfigSetID attribute of the Device.
     *
     * @return The current refConfigSetID
     */
    public Integer getRefConfigSetID() {
        return this.refConfigSetID;
    }


    /**
     * sets the refConfigurationSetID for this Device.
     *
     * @param refConfigSetID  the PK of the RefConfigurationSet that
     * you wish to assocatied with this Device.
     */
    public void setRefConfigSetID(Integer refConfigSetID) {
        this.refConfigSetID = refConfigSetID;
    }


    /**
     * sets the coreSoftwareDetailsID for this Device.
     *
     * @param  coreSoftwareDetailsID  the primary key of coreSoftwareDetails
     * (version of OS or firmware to run on the LogicalPhone) that you wish
     * to associate with the Device
     */
    public void setCoreSoftwareDetailsID(Integer coreSoftwareDetailsID) {
        this.coreSoftwareDetailsID = coreSoftwareDetailsID;
    }


    /**
     * gets the coreSoftwareDetailsID for this Device.
     *
     * @return the PK of the CoreSoftwareDetails currently
     * asssociated with the LogicalPhone.
     */
    public Integer getCoreSoftwareDetailsID() {
        return coreSoftwareDetailsID;
    }


    /**
     * sets the description for this Device.
     *
     * @param description some free-text that you want to use to label this
     * Device.
     */
    public void setDescription(String description) throws PDSException {
        if (description != null && description.length() > MAX_DESCRIPTION_LEN ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3028",
                                        new Object[]{
                                            "description",
                                            new Integer(MAX_DESCRIPTION_LEN)}));
        }
        this.description = description;
    }


    /**
     * gets the description for this Device.
     *
     *@return the description of this Device.
     */
    public String getDescription() {
        return description;
    }


    /**
     * gets the deviceTypeID for this Device.
     *
     * @return the deviceTypeID of this Device.
     */
    public Integer getDeviceTypeID() {
        return this.deviceTypeID;
    }


    /**
     * gets the serialNumber for this Device.
     *
     * @return the serialNumber of this Device.
     */
    public String getSerialNumber() {
        return serialNumber;
    }


    /**
     * setSerialNumber sets the serialNumber for this Device.
     *
     * @param serialNumber the new serialNumber you wish to assign to this Device.
     *
     * @throws PDSException is thrown for a variety of validation errors such as using
     * a serialNumber associated with another phone, the serialNumber being too long
     * or containing invalid characters.
     */
    public void setSerialNumber (String serialNumber) throws PDSException {

        // serialNumber must not be null
        if(serialNumber == null){
            mCTX.setRollbackOnly();
            throw new PDSException(collateErrorMessages("E3038", null));
        }

        if (serialNumber != null && serialNumber.length() > MAX_SERIAL_NUMBER_LEN ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E3028",
                                        new Object[]{
                                            "serial number",
                                            new Integer(MAX_SERIAL_NUMBER_LEN)}));
        }

        // check that the serialNumber is a valid MAC address.
        if (!SERIAL_NUMBER_RE.match(serialNumber)) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E3035", new Object[]{serialNumber}));
        }

        Collection existing = null;
        try {
            existing = ((DeviceHome) mCTX.getEJBHome()).findBySerialNumber(serialNumber);
        }
        catch (Exception e) {
            logFatal (e.getMessage(), e);
            throw new EJBException (e.getMessage());
        }

        if (!existing.isEmpty()) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E3029", new Object [] {serialNumber}));
        }

        // Cisco phones must have uppercased MAC/serial #, their limitation
        // not ours.
        String model = getModel();

        if ( model.equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 ) ||
                model.equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 )) {
            serialNumber = serialNumber.toUpperCase();
        }

        this.serialNumber = serialNumber;
    }


    /**
     * returns a 'friendly' identity of the Device.  This is userful for identifying
     * the Device to users of the system.
     *
     * @return friendly identity/description of the Device.
     */
    public String getExternalID () {
        Organization organization = null;
        try {
            organization = mOrganizationHome.findByPrimaryKey( organizationID );
            return "name: " + this.shortName +
                " organization: " + organization.getExternalID();
        }
        catch ( Exception ex ) {
            logFatal( ex.toString(), ex  );
            throw new EJBException ( ex );
        }
    }


    /**
     * TODO: should be removed
     * calculateDeviceLineURL works out the 'DEVICE_LINE' SIP URL
     * for Pingtel Devices.
     *
     * @return a String value of the Device Line SIP URL.
     * @throws PDSException is thrown for application level errors.
     */
    public String calculateDeviceLineURL () throws PDSException {
        String url = null;

        try {
            Organization org = getOrganization();

            url = this.getShortName() + "<sip:" + this.getSerialNumber() + "@" +
                org.getDNSDomain() + ">";
        }
        catch ( RemoteException ex ) {
            throw new EJBException ( ex.toString() );
        }

        return url;
    }


    /**
     * returns the model for this devices device type.
     * @return returns the model for this devices device type
     */
    public String getModel () {
        try {
            return getDeviceTypeEJBObject().getModel();
        }
        catch (RemoteException e) {
            logFatal ( e.toString(), e );
            throw new EJBException ( e.toString() );
        }
    }



    /**
     * returns the manufacturer name of this devices
     * @return manufacturer name of this devices
     */
    public String getManufaturerName () {
        try {
            return getManufacturerEJBObject().getName();
        }
        catch (RemoteException e) {
            logFatal ( e.toString(), e );
            throw new EJBException  ( e.toString() );
        }
    }

    /**
     * getOrganization returns the Organization EJBObject of the Organization
     * that this Device belongs to.
     *
     * @return Organization EJBObject which owns this Device.
     * @throws PDSException is thrown for application level errors.
     */
    public Organization getOrganization() throws PDSException {
        if ( mOrganizationEJBObject == null ) {
            try {
                mOrganizationEJBObject =
                        mOrganizationHome.findByPrimaryKey( getOrganizationID() );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E1018",
                                            new Object[]{ getOrganizationID() }),
                    ex);
            }
            catch ( RemoteException ex ) {
                logFatal ( ex.toString(), ex  );

                throw new EJBException( ex.toString());
            }
        }

        return mOrganizationEJBObject;
    }




//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * sets the PK (id) for this Device.  This is immutable.   This will
     * need to be made private for EJB 2.X.
     * @param id
     */
    public void setId(Integer id) {
        this.id = id;
    }


    private void setOrganizationID(Integer organizationID) {
           this.organizationID = organizationID;
    }

    /**
     * set the deviceTypeID for this Device.
     *
     * @param deviceTypeID the PK for the DeviceType that you want to associate
     * with this Device.
     */
    private void setDeviceTypeID(Integer deviceTypeID) {
        this.deviceTypeID = deviceTypeID;
    }

    private DeviceType getDeviceTypeEJBObject () {
        if ( mDeviceTypeEJBObject == null ) {
            try {
                mDeviceTypeEJBObject = mDeviceTypeHome.findByPrimaryKey( getDeviceTypeID() );

            }
            catch ( Exception e) {
                logFatal ( e.toString(), e );
                throw new EJBException  ( e.toString() );
            }
        }

        return mDeviceTypeEJBObject;
    }


    private Manufacturer getManufacturerEJBObject () {
        if ( mManufacturerEJBObject == null ) {
            try {
                mManufacturerEJBObject =
                        mManufacturerHome.findByPrimaryKey(
                                getDeviceTypeEJBObject().getManufacturerID() );
            }
            catch ( Exception e) {
                logFatal ( e.toString(), e );
                throw new EJBException  ( e.toString() );
            }
        }

        return mManufacturerEJBObject;
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////



}

