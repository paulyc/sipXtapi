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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.apache.commons.lang.ArrayUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * MappingRulesTest
 */
public class FallbackRulesTest extends XMLTestCase {
    public FallbackRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerateRuleWithGateways() throws Exception {
        Gateway g1 = new Gateway();
        g1.setAddress("10.1.1.14");
        FullTransform t1 = new FullTransform();
        t1.setUser("333");
        t1.setHost(g1.getAddress());
        t1.setFieldParams(new String[] {
            "Q=0.97"
        });

        IMocksControl control = EasyMock.createStrictControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.isInternal();
        control.andReturn(false);
        rule.getHostPatterns();
        control.andReturn(ArrayUtils.EMPTY_STRING_ARRAY);        
        rule.getName();
        control.andReturn("my test name");
        rule.getDescription();
        control.andReturn("my test description");
        rule.getPatterns();
        control.andReturn(new String[] {
            "x."
        });
        rule.isInternal();
        control.andReturn(false);
        rule.getTransforms();
        control.andReturn(new Transform[] {
            t1
        });
        control.replay();

        MappingRules mappingRules = new FallbackRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        String domDoc = XmlUnitHelper.asString(document);

        assertXpathEvaluatesTo("my test description",
                "/mappings/hostMatch/userMatch/description", domDoc);
        assertXpathEvaluatesTo("x.", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);
        assertXpathEvaluatesTo("333",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/user", domDoc);
        assertXpathEvaluatesTo(g1.getAddress(),
                "/mappings/hostMatch/userMatch/permissionMatch/transform/host", domDoc);
        assertXpathEvaluatesTo("Q=0.97",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/fieldparams", domDoc);

        control.verify();
    }

    public void testGenerateRuleWithoutGateways() throws Exception {
        IMocksControl control = EasyMock.createControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.isInternal();
        control.andReturn(true);
        control.replay();

        MappingRules mappingRules = new FallbackRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        String domDoc = XmlUnitHelper.asString(document);

        assertXpathNotExists("/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch", domDoc);
        assertXpathExists("/mappings/hostMatch/hostPattern", domDoc);

        control.verify();
    }
}