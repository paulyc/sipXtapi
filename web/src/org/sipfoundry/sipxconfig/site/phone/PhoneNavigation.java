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
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Top portion of pages that show tabs, help box, intro text, etc
 */
public abstract class PhoneNavigation extends BaseComponent {
    
    /** REQUIRED PARAMETER */
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract void setSettingGroup(SettingGroup model);
    
    public abstract SettingGroup getSettingGroup();

    public abstract void setCurrentSettingGroup(SettingGroup setting);

    public abstract SettingGroup getCurrentSettingGroup();
    
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setPhoneId(endpointId.intValue());

        cycle.activate(page);
    }
    
    public void editLines(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setPhoneId(endpointId.intValue());
        
        cycle.activate(page);
    }
    
    public void editSettings(IRequestCycle cycle) {
        PhoneSettings page = (PhoneSettings) cycle.getPage(PhoneSettings.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setPhoneId(endpointId.intValue());

        String section = (String) TapestryUtils.assertParameter(String.class, params, 1);        
        page.setParentSettingGroupName(section);
        
        cycle.activate(page);        
    }
    
    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] { 
            new Integer(getPhone().getEndpoint().getId()),
            getCurrentSettingGroup().getName() 
        };
    }
    
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        SettingGroup model = getPhone().getSettingGroup();
        setSettingGroup(model);
    }    
}
