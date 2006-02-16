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
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Decorates property selection model but inserting a new value as the first value in the model.
 */
public class ExtraOptionModelDecorator implements IPropertySelectionModel {
    private static final Integer RESERVED = new Integer(-1);

    /** decorated model */
    private IPropertySelectionModel m_model;

    private Object m_extraOption = RESERVED;

    private String m_extraLabel = StringUtils.EMPTY;

    public String getLabel(int index) {
        if (index == 0) {
            return m_extraLabel;
        }
        return m_model.getLabel(index - 1);
    }

    public Object getOption(int index) {
        if (index == 0) {
            return m_extraOption;
        }
        return m_model.getOption(index - 1);
    }

    public int getOptionCount() {
        return m_model.getOptionCount() + 1;
    }

    public String getValue(int index) {
        if (index == 0) {
            return ObjectUtils.toString(m_extraOption);
        }
        return m_model.getValue(index - 1);
    }

    public Object translateValue(String value) {
        if (ObjectUtils.equals(value, ObjectUtils.toString(m_extraOption))) {
            return m_extraOption;
        }
        return m_model.translateValue(value);
    }

    public void setExtraLabel(String extraLabel) {
        m_extraLabel = extraLabel;
    }

    public void setExtraOption(Object extraOption) {
        m_extraOption = extraOption;
    }

    public void setModel(IPropertySelectionModel model) {
        m_model = model;
    }

    public ExtraOptionModelDecorator decorate(IPropertySelectionModel model) {
        setModel(model);
        return this;
    }
}
