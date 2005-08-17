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
package org.sipfoundry.sipxconfig.setting;

import java.util.Iterator;
import java.util.Set;
import java.util.TreeSet;

/**
 * Common code for line and phone information.
 */
public class BeanWithGroups extends BeanWithSettings {

    private Set m_groups = new TreeSet();
    
    public Set getGroups() {
        return m_groups;
    }

    public void setGroups(Set settingSets) {
        m_groups = settingSets;        
    }
        
    public void addGroup(Group tag) {
        m_groups.add(tag);
    }

    protected Setting decorateSettings(Setting settings) {
        Set groups = getGroups();
        if (groups != null) {
            Iterator i = groups.iterator();
            while (i.hasNext()) {
                SettingVisitor visitor = (SettingVisitor) i.next(); 
                settings.acceptVisitor(visitor);
            }
        }
        
        Setting decorated = super.decorateSettings(settings);
        
        return decorated;    
    }
}
