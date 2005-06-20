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

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public abstract class AbstractStorage extends BeanWithId implements Storage {

    private Map m_delegate = new HashMap();

    public Object get(Object key) {
        return m_delegate.get(key);
    }

    public Object put(Object key, Object value) {
        return m_delegate.put(key, value);
    }

    public Object remove(Object key) {
        return m_delegate.remove(key);
    }

    protected Map getDelegate() {
        return m_delegate;
    }

    protected void setDelegate(Map delegate) {
        m_delegate = delegate;
    }

    public Object getValue(Setting setting) {
        return get(setting.getPath());
    }

    public Object setValue(Setting setting, Object value) {
        return put(setting.getPath(), value);
    }

    public Object remove(Setting setting) {
        return remove(setting.getPath());
    }

    public int size() {
        return m_delegate.size();
    }    
}
