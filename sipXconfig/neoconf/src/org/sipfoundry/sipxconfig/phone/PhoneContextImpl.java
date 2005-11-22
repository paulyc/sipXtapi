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
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware,
        PhoneContext, ApplicationListener, DaoEventListener {

    private static final String GROUP_RESOURCE_ID = "phone";
    private static final String QUERY_PHONE = "from Phone";
    private static final String QUERY_PHONE_ID_BY_SERIAL_NUMBER = "phoneIdsWithSerialNumber";

    private CoreContext m_coreContext;

    private SettingDao m_settingDao;

    private BeanFactory m_beanFactory;

    private List m_availableModels;
    
    private List m_firmwareManufacturers;

    private JobQueue m_jobQueue;

    private String m_systemDirectory;

    private Map m_modelCache = new HashMap();

    private PhoneDefaults m_phoneDefaults;

    /**
     * Generate profile on phones in background
     */
    public void generateProfilesAndRestart(Collection phones) {
        JobRecord job = createJobRecord(phones, JobRecord.TYPE_PROJECTION);
        m_jobQueue.addJob(job);
    }

    public void generateProfilesAndRestartAll() {
        Collection phones = getHibernateTemplate().loadAll(Phone.class);
        generateProfilesAndRestart(phones);
    }

    public List getAvailablePhoneModels() {
        return m_availableModels;
    }

    public void setAvailablePhoneModels(List models) {
        m_availableModels = models;
    }

    /**
     * Restart phones in background
     */
    public void restart(Collection phones) {
        JobRecord job = createJobRecord(phones, JobRecord.TYPE_DEVICE_RESTART);
        m_jobQueue.addJob(job);
    }

    JobRecord createJobRecord(Collection phones, int type) {
        JobRecord job = new JobRecord();
        job.setType(type);
        Phone[] phonesArray = (Phone[]) phones.toArray(new Phone[0]);
        job.setPhones(phonesArray);

        return job;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setJobQueue(JobQueue jobQueue) {
        m_jobQueue = jobQueue;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public void storePhone(Phone phone) {
        HibernateTemplate hibernate = getHibernateTemplate();
        String serialNumber = phone.getSerialNumber();
        DaoUtils.checkDuplicatesByNamedQuery(hibernate, phone, QUERY_PHONE_ID_BY_SERIAL_NUMBER,
                serialNumber, new DuplicateSerialNumberException(serialNumber));
        phone.setValueStorage(clearUnsavedValueStorage(phone.getValueStorage()));
        hibernate.saveOrUpdate(phone);
    }

    public void deletePhone(Phone phone) {
        phone.removeProfiles();
        phone.setValueStorage(clearUnsavedValueStorage(phone.getValueStorage()));
        Iterator i = phone.getLines().iterator();
        while (i.hasNext()) {
            Line line = (Line) i.next();
            line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        }
        getHibernateTemplate().delete(phone);
    }

    public void storeLine(Line line) {
        line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        getHibernateTemplate().saveOrUpdate(line);
    }

    public void deleteLine(Line line) {
        line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        getHibernateTemplate().delete(line);
    }

    ValueStorage clearUnsavedValueStorage(ValueStorage vs) {
        // If no settings don't bother saving anything.
        return vs != null && vs.isNew() && vs.size() == 0 ? null : vs;
    }

    public Line loadLine(Integer id) {
        Line line = (Line) getHibernateTemplate().load(Line.class, id);
        return line;
    }

    public int getPhonesCount() {
        return getPhonesInGroupCount(null);
    }

    public int getPhonesInGroupCount(Integer groupId) {
        return getBeansInGroupCount(Phone.class, groupId);
    }

    public List loadPhonesByPage(Integer groupId, int firstRow, int pageSize, String orderBy,
            boolean orderAscending) {
        return loadBeansByPage(Phone.class, groupId, firstRow, pageSize, orderBy, orderAscending);
    }

    public Collection loadPhones() {
        String phoneQuery = QUERY_PHONE;
        return getHibernateTemplate().find(phoneQuery);
    }

    public Phone loadPhone(Integer id) {
        Phone phone = (Phone) getHibernateTemplate().load(Phone.class, id);
        
        return phone;
    }

    public Integer getPhoneIdBySerialNumber(String serialNumber) {
        Integer phoneId = null;
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_PHONE_ID_BY_SERIAL_NUMBER, "value", serialNumber);
        if (SipxCollectionUtils.safeSize(objs) != 0) {
            if (objs.size() > 1) {
                // There is a database uniqueness constraint that should prevent this from ever
                // happening
                throw new IllegalStateException("Duplicate phone serial number: " + serialNumber);
            }
            phoneId = (Integer) objs.get(0);
        }
        return phoneId;
    }

    public Phone newPhone(PhoneModel model) {
        Phone phone = (Phone) m_beanFactory.getBean(model.getBeanId());
        phone.setModelId(model.getModelId());

        return phone;
    }

    public List getGroups() {
        return m_settingDao.getGroups(GROUP_RESOURCE_ID);
    }

    /** unittesting only */
    public void clear() {
        // ordered bottom-up, e.g. traverse foreign keys so as to
        // not leave hanging references. DB will reject otherwise
        deleteAll(QUERY_PHONE);
        deleteAll("from Group where resource = 'phone'");
    }

    private void deleteAll(String query) {
        Collection c = getHibernateTemplate().find(query);
        getHibernateTemplate().deleteAll(c);
    }

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }

    public Setting getSettingModel(String filename) {
        // cache it, but may be helpful to reload model on fly in future
        Setting model = (Setting) m_modelCache.get(filename);
        if (model == null) {
            File modelDefsFile = new File(getSystemDirectory() + '/' + filename);
            model = new XmlModelBuilder(getSystemDirectory()).buildModel(modelDefsFile);
            m_modelCache.put(filename, model);
        }
        return model;
    }

    private class DuplicateSerialNumberException extends UserException {
        private static final String ERROR = "A phone with serial number: {0} already exists.";

        public DuplicateSerialNumberException(String serialNumber) {
            super(ERROR, serialNumber);
        }
    }

    public void onApplicationEvent(ApplicationEvent event_) {
        // no init tasks defined yet
    }

    public PhoneDefaults getPhoneDefaults() {
        return m_phoneDefaults;
    }

    public void setPhoneDefaults(PhoneDefaults phoneDefaults) {
        m_phoneDefaults = phoneDefaults;
    }

    public Collection getPhonesByGroupId(Integer groupId) {
        Collection users = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "phonesByGroupId", "groupId", groupId);
        return users;
    }

    public void onDelete(Object entity) {
        Class c = entity.getClass();
        if (Group.class.equals(c)) {
            Group group = (Group) entity;
            getHibernateTemplate().update(group);
            if (Phone.GROUP_RESOURCE_ID.equals(group.getResource())) {
                Collection phones = getPhonesByGroupId(group.getId());
                Iterator iphones = phones.iterator();
                while (iphones.hasNext()) {
                    Phone phone = (Phone) iphones.next();
                    Object[] ids = new Object[] {
                        group.getId()
                    };
                    DataCollectionUtil.removeByPrimaryKey(phone.getGroups(), ids);
                    storePhone(phone);
                }
            }
        } else if (User.class.equals(c)) {
            User user = (User) entity;
            Collection phones = getPhonesByUserId(user.getId());
            Iterator iphones = phones.iterator();
            while (iphones.hasNext()) {
                Phone phone = (Phone) iphones.next();
                Collection lines = phone.getLines();
                Iterator ilines = lines.iterator();
                List ids = new ArrayList();
                while (ilines.hasNext()) {
                    Line line = (Line) ilines.next();
                    User lineUser = line.getUser();
                    if (lineUser.getId().equals(user.getId())) {
                        ids.add(line.getId());
                    }
                }
                DataCollectionUtil.removeByPrimaryKey(lines, ids.toArray());
                storePhone(phone);
            }
        }
    }

    public void onSave(Object entity_) {
    }

    public Collection getPhonesByUserId(Integer userId) {
        Collection users = getHibernateTemplate().findByNamedQueryAndNamedParam("phonesByUserId",
                "userId", userId);
        return users;
    }

    public void addToGroup(Integer groupId, Collection ids) {
        DaoUtils.addToGroup(getHibernateTemplate(), groupId, Phone.class, ids);
    }

    public void removeFromGroup(Integer groupId, Collection ids) {
        DaoUtils.removeFromGroup(getHibernateTemplate(), groupId, Phone.class, ids);
    }

    public void addUsersToPhone(Integer phoneId, Collection ids) {
        Phone phone = loadPhone(phoneId);
        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer userId = (Integer) i.next();
            User user = m_coreContext.loadUser(userId);
            Line line = phone.createLine();
            line.setUser(user);
            phone.addLine(line);
        }
        storePhone(phone);
    }

    public Firmware loadFirmware(Integer firmwareId) {
        return (Firmware) getHibernateTemplate().load(Firmware.class, firmwareId);
    }

    public void saveFirmware(Firmware firmware) {
        getHibernateTemplate().save(firmware);
    }

    public void deleteFirmware(Firmware firmware) {
        getHibernateTemplate().delete(firmware);
    }

    public List getFirmwareManufacturers() {
        return m_firmwareManufacturers;
    }
    
    public void setFirmwareManufacturers(List firmwareManufacturers) {
        m_firmwareManufacturers = firmwareManufacturers;
    }
    
    public List getFirmware() {
        return getHibernateTemplate().findByNamedQuery("firmware");
    }
}
