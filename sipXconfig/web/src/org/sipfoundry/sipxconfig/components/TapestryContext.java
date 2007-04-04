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
package org.sipfoundry.sipxconfig.components;

import java.text.DateFormat;
import java.util.Collection;
import java.util.Locale;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.ApplicationRuntimeException;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Tapestry utilities available to web pages
 */
public class TapestryContext {
    public static final String CONTEXT_BEAN_NAME = "tapestry";

    private HivemindContext m_hivemindContext;

    public void setHivemindContext(HivemindContext hivemindContext) {
        m_hivemindContext = hivemindContext;
    }

    public HivemindContext getHivemindContext() {
        return m_hivemindContext;
    }

    /**
     * Add a option to the dropdown model with a label to instruct the user to make a selection.
     * If not item is selected, your business object method will be explicitly set to null
     */
    public IPropertySelectionModel instructUserToSelect(IPropertySelectionModel model,
            Messages messages) {
        return addExtraOption(model, messages, "prompt.select");
    }

    /**
     * Add a option to the dropdown model with a label to instruct the user to make a selection.
     * If not item is selected, your business object method will be explicitly set to null
     */
    public IPropertySelectionModel addExtraOption(IPropertySelectionModel model,
            Messages messages, String extraKey) {
        ExtraOptionModelDecorator decorated = new ExtraOptionModelDecorator();
        decorated.setExtraLabel(messages.getMessage(extraKey));
        decorated.setExtraOption(null);
        decorated.setModel(model);

        return decorated;
    }
    
    /**
     * Translates UserExceptions into form errors instead redirecting to an error page.
     */
    public IActionListener treatUserExceptionAsValidationError(IValidationDelegate validator,
            IActionListener listener) {
        return new UserExceptionAdapter(validator, listener);
    }

    static class UserExceptionAdapter implements IActionListener {

        private IActionListener m_listener;

        private IValidationDelegate m_validator;

        UserExceptionAdapter(IValidationDelegate validator, IActionListener listener) {
            m_listener = listener;
            m_validator = validator;
        }

        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            try {
                m_listener.actionTriggered(component, cycle);
            } catch (ApplicationRuntimeException are) {
                UserException cause = getUserExceptionCause(are);
                if (cause != null) {
                    recordUserException(cause);
                } else {
                    throw are;
                }
            } catch (UserException ue) {
                recordUserException(ue);
            }
        }

        /**
         * Starting with Tapestry 4, Listeners wrap exceptions with ApplicationRuntimeException.
         * We have to prepare for many levels of exceptions as listeners are often wrapped by
         * other listeners
         */
        UserException getUserExceptionCause(ApplicationRuntimeException e) {
            Throwable t = e.getCause();
            if (t instanceof UserException) {
                return (UserException) t;
            }
            if (t instanceof ApplicationRuntimeException && t != e) {
                // recurse
                return getUserExceptionCause((ApplicationRuntimeException) t);
            }
            return null;
        }

        private void recordUserException(UserException e) {
            m_validator.record(new ValidatorException(e.getMessage()));
        }
    }

    /**
     * Join a list of names objects into a string give a delimitor Example <span jwcid="@Insert"
     * value="ognl:tapestry.joinNamed(items, ', ')"/>
     */
    public String joinNamed(Collection namedItems, String delim) {
        Collection names = CollectionUtils.collect(namedItems, new NamedObject.ToName());
        return StringUtils.join(names.iterator(), delim);
    }
    
    /**
     * Example:
     *  <span jwcid="@Insert" value="someDate" format="tapestry.date(locale)"/>
     */
    public DateFormat date(Locale locale) {
        return TapestryUtils.getDateFormat(locale);
    }
}