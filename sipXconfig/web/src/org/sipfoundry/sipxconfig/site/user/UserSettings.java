/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class UserSettings extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "UserSettings";

    @Persist
    public abstract void setUserId(Integer userId);

    public abstract Integer getUserId();

    public abstract User getUser();

    public abstract void setUser(User user);

    @Persist
    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String settingName);

    /** REQUIRED PAGE PARAMETER */
    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user != null) {
            return;
        }

        user = getCoreContext().loadUser(getUserId());
        setUser(user);
        Setting root = user.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    public String ok() {
        apply();
        return ManageUsers.PAGE;
    }

    public void apply() {
        CoreContext dao = getCoreContext();
        dao.saveUser(getUser());
    }

    public String cancel() {
        return ManageUsers.PAGE;
    }

    public String getParentSettingLabel() {
        Setting setting = getParentSetting();
        return TapestryUtils.getModelMessage(this, setting.getMessageSource(), setting
                .getLabelKey(), StringUtils.EMPTY);
    }

    public String getParentSettingDescription() {
        Setting setting = getParentSetting();
        return TapestryUtils.getModelMessage(this, setting.getMessageSource(), setting
                .getDescriptionKey(), StringUtils.EMPTY);
    }
}