/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.List;

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl extends SipxHibernateDaoSupport implements LdapManager, ApplicationContextAware {
    public static final String FILTER_ALL_CLASSES = "objectclass=*";

    public static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);

    private JndiLdapTemplate m_jndiTemplate;

    private ApplicationContext m_applicationContext;

    public void verify(LdapConnectionParams params, AttrMap attrMap) {
        params.applyToTemplate(m_jndiTemplate);

        try {
            String searchBase = retrieveDefaultSearchBase();
            // it will only overwrite the search base if not set yet
            if (StringUtils.isBlank(attrMap.getSearchBase())) {
                attrMap.setSearchBase(searchBase);
            }
        } catch (NamingException e) {
            LOG.debug("Verifying LDAP connection failed.", e);
            throw new UserException("Cannot connect to LDAP server: " + e.getMessage());
        }
    }

    public Schema getSchema() {
        getConnectionParams().applyToTemplate(m_jndiTemplate);
        try {
            return retrieveSchema();
        } catch (NamingException e) {
            LOG.debug("Retrieving schema failed.", e);
            throw new UserException("Cannot retrieve schema from LDAP server: " + e.getMessage());
        }
    }

    /**
     * Connects to LDAP to retrieve the namingContexts attribute from root. Good way to verify if
     * LDAP is accessible. Command line anologue is:
     * 
     * ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
     * 
     * @return namingContext value - can be used as the search base for user if nothing more
     *         specific is provided
     * @throws NamingException
     */
    private String retrieveDefaultSearchBase() throws NamingException {
        SearchControls cons = new SearchControls();
        String[] attrs = new String[] {
            "namingContexts"
        };

        cons.setReturningAttributes(attrs);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);
        NamingEnumeration<SearchResult> results = m_jndiTemplate.search("", FILTER_ALL_CLASSES, cons);
        // only interested in the first result
        if (results.hasMore()) {
            SearchResult result = results.next();
            return (String) result.getAttributes().get(attrs[0]).get();
        }
        return StringUtils.EMPTY;
    }

    private Schema retrieveSchema() throws NamingException {
        SearchControls cons = new SearchControls();
        String[] attrs = new String[] {
            "objectClasses"
        };

        cons.setReturningAttributes(attrs);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);
        NamingEnumeration<SearchResult> results = m_jndiTemplate.search("cn=subSchema", FILTER_ALL_CLASSES,
                cons);
        // only interested in the first result

        Schema schema = new Schema();
        if (!results.hasMore()) {
            return schema;
        }
        SearchResult result = results.next();
        NamingEnumeration definitions = result.getAttributes().get(attrs[0]).getAll();
        while (definitions.hasMoreElements()) {
            String classDefinition = (String) definitions.nextElement();
            schema.addClassDefinition(classDefinition);
        }
        return schema;
    }

    public CronSchedule getSchedule() {
        return getConnectionParams().getSchedule();
    }

    public void setSchedule(CronSchedule schedule) {
        if (!schedule.isNew()) {
            // XCF-1168 incoming schedule is probably an update to this schedule
            // so add this schedule to cache preempt hibernate error about multiple
            // objects with same ID in session. This is only true because schedule
            // is managed by LdapConnectionParams object.
            getHibernateTemplate().update(schedule);
        }

        LdapConnectionParams connectionParams = getConnectionParams();
        connectionParams.setSchedule(schedule);
        getHibernateTemplate().update(connectionParams);

        m_applicationContext.publishEvent(new LdapImportTrigger.ScheduleChangedEvent(schedule, this));
    }

    public AttrMap getAttrMap() {
        List<AttrMap> connections = getHibernateTemplate().loadAll(AttrMap.class);
        if (!connections.isEmpty()) {
            return connections.get(0);
        }
        AttrMap attrMap = (AttrMap) m_applicationContext.getBean("attrMap", AttrMap.class);
        getHibernateTemplate().save(attrMap);
        return attrMap;
    }

    public LdapConnectionParams getConnectionParams() {
        List<LdapConnectionParams> connections = getHibernateTemplate().loadAll(LdapConnectionParams.class);
        if (!connections.isEmpty()) {
            return connections.get(0);
        }
        LdapConnectionParams params = (LdapConnectionParams) m_applicationContext.getBean(
                "ldapConnectionParams", LdapConnectionParams.class);
        getHibernateTemplate().save(params);
        return params;
    }

    public void setAttrMap(AttrMap attrMap) {
        getHibernateTemplate().saveOrUpdate(attrMap);
    }

    public void setConnectionParams(LdapConnectionParams params) {
        getHibernateTemplate().saveOrUpdate(params);
    }

    public void setJndiTemplate(JndiLdapTemplate jndiTemplate) {
        m_jndiTemplate = jndiTemplate;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }
}