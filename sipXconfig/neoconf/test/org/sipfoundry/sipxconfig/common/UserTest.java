/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

public class UserTest extends TestCase {
    private static final String USERNAME = "username";
    private static final String PIN = "pin";
    private static final String SIP_PASSWORD = "sip password";
    private static final String REALM = "sipfoundry.org";
    private static final String DOMAIN = "sipfoundry.org";

    public void testGetDisplayName() {
        User u = new User();
        assertNull(u.getDisplayName());
        u.setUserName("bob");
        assertNull(u.getDisplayName());
        u.setFirstName("First");
        assertEquals("First", u.getDisplayName());
        u.setLastName("Last");
        assertEquals("First Last", u.getDisplayName());
    }

    public void testGetUri() {
        User user = new User();
        user.setUserName(USERNAME);
        String uri = user.getUri("mycomp.com");

        assertEquals("sip:" + USERNAME + "@mycomp.com", uri);

        user.setLastName("Last");
        uri = user.getUri("mycomp.com");
        assertEquals("Last<sip:" + USERNAME + "@mycomp.com>", uri);

        user.setFirstName("First");
        uri = user.getUri("mycomp.com");
        assertEquals("First Last<sip:" + USERNAME + "@mycomp.com>", uri);
    }

    /** Test that setting a typical PIN yields expected results */
    public void testSetPin() throws Exception {
        checkSetPin(PIN);
    }

    /** Test that setting a null PIN yields expected results */
    public void testSetNullPin() throws Exception {
        checkSetPin(null);
    }
    
    private void checkSetPin(String pin) throws Exception {
        User user = new User();
        user.setUserName(USERNAME);
        user.setPin(pin, REALM);
        String pintoken = getPintoken(USERNAME, pin);
        assertEquals(pintoken, user.getPintoken());
    }

    public void testGetSipPasswordHash() throws Exception {
        User user = new User();
        user.setUserName(USERNAME);
        user.setSipPassword(SIP_PASSWORD);
        String hash = Md5Encoder.digestPassword(USERNAME, REALM, SIP_PASSWORD);

        assertEquals(hash, user.getSipPasswordHash(REALM));
    }

    public void testGetSipPasswordHashEmpty() throws Exception {
        User user = new User();
        user.setUserName(USERNAME);
        user.setSipPassword(null);
        String hash = Md5Encoder.digestPassword(USERNAME, REALM, "");

        assertEquals(hash, user.getSipPasswordHash(REALM));
    }

    public void testGetSipPasswordHashMd5() throws Exception {
        User user = new User();
        user.setUserName(USERNAME);
        String hash = Md5Encoder.digestPassword(USERNAME, REALM, "");
        user.setSipPassword(hash);

        String newHash = Md5Encoder.digestPassword(USERNAME, REALM, hash);

        assertFalse(hash.equals(newHash));
        assertEquals(newHash, user.getSipPasswordHash(REALM));
    }
    
    public void testGetAliases() {
        User user = new User();
        user.setUserName(USERNAME);
        user.setExtension("4444");
        List aliases = user.getAliases(DOMAIN);
        assertEquals(1, aliases.size());
        AliasMapping alias = (AliasMapping) aliases.get(0);
        assertEquals("4444@sipfoundry.org", alias.getIdentity());
        assertEquals("sip:" + USERNAME + "@" + DOMAIN, alias.getContact());
    }
    
    public void testGetAliasesNoExtension() {
        User user = new User();
        user.setUserName(USERNAME);
        user.setExtension(null);
        List aliases = user.getAliases(DOMAIN);
        assertEquals(0, aliases.size());
    }
    
    public void testHasPermission() {
        User user = new User();
        user.setSettingModel(TestHelper.loadSettings("user-settings.xml"));
        
        Group group = new Group();
        user.addGroup(group);
        
        Setting superAdmin = user.getSettings().getSetting(Permission.SUPERADMIN.getSettingPath()); 
        assertNotNull(superAdmin);
        assertFalse(user.hasPermission(Permission.SUPERADMIN));
    }

    
    private String getPintoken(String username, String pin) {
        pin = (String) ObjectUtils.defaultIfNull(pin, StringUtils.EMPTY);   // handle null pin
        return Md5Encoder.digestPassword(username, REALM, pin);
    }
}
