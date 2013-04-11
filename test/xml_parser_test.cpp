/*
* Copyright (c) 2008, AMT – The Association For Manufacturing Technology (“AMT”)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the AMT nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* DISCLAIMER OF WARRANTY. ALL MTCONNECT MATERIALS AND SPECIFICATIONS PROVIDED
* BY AMT, MTCONNECT OR ANY PARTICIPANT TO YOU OR ANY PARTY ARE PROVIDED "AS IS"
* AND WITHOUT ANY WARRANTY OF ANY KIND. AMT, MTCONNECT, AND EACH OF THEIR
* RESPECTIVE MEMBERS, OFFICERS, DIRECTORS, AFFILIATES, SPONSORS, AND AGENTS
* (COLLECTIVELY, THE "AMT PARTIES") AND PARTICIPANTS MAKE NO REPRESENTATION OR
* WARRANTY OF ANY KIND WHATSOEVER RELATING TO THESE MATERIALS, INCLUDING, WITHOUT
* LIMITATION, ANY EXPRESS OR IMPLIED WARRANTY OF NONINFRINGEMENT,
* MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 

* LIMITATION OF LIABILITY. IN NO EVENT SHALL AMT, MTCONNECT, ANY OTHER AMT
* PARTY, OR ANY PARTICIPANT BE LIABLE FOR THE COST OF PROCURING SUBSTITUTE GOODS
* OR SERVICES, LOST PROFITS, LOSS OF USE, LOSS OF DATA OR ANY INCIDENTAL,
* CONSEQUENTIAL, INDIRECT, SPECIAL OR PUNITIVE DAMAGES OR OTHER DIRECT DAMAGES,
* WHETHER UNDER CONTRACT, TORT, WARRANTY OR OTHERWISE, ARISING IN ANY WAY OUT OF
* THIS AGREEMENT, USE OR INABILITY TO USE MTCONNECT MATERIALS, WHETHER OR NOT
* SUCH PARTY HAD ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
*/

#include "xml_parser_test.hpp"
#include "test_globals.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(XmlParserTest);

using namespace std;

void XmlParserTest::setUp()
{
  a = NULL;
  try
  {
    a = new XmlParser();
    mDevices = a->parseFile("../samples/test_config.xml");
  }
  catch (exception & e)
  {
    CPPUNIT_FAIL("Could not locate test xml: ../samples/test_config.xml");
  }
}

void XmlParserTest::tearDown()
{
  if (a != NULL)
    delete a;
}

void XmlParserTest::testConstructor()
{
  a = new XmlParser();
  CPPUNIT_ASSERT_THROW(a->parseFile("../samples/badPath.xml"), std::runtime_error);
  delete a;
  a = new XmlParser();
  CPPUNIT_ASSERT_NO_THROW(a->parseFile("../samples/test_config.xml"));
}

void XmlParserTest::testGetDevices()
{
  CPPUNIT_ASSERT_EQUAL((size_t) 1, mDevices.size());

  Device *device = mDevices.front();
  
  // Check for Description
  CPPUNIT_ASSERT_EQUAL((string) "Linux CNC Device", device->getDescriptionBody());
  
  vector<DataItem*> dataItems;
  std::map<string, DataItem *> dataItemsMap = device->getDeviceDataItems();  
  std::map<string, DataItem *>::iterator iter;
  for (iter = dataItemsMap.begin(); iter != dataItemsMap.end(); iter++)
  {
    dataItems.push_back(iter->second);
  }
    
  bool hasExec = false, hasZcom = false;
  
  vector<DataItem *>::iterator dataItem;
  for (dataItem = dataItems.begin(); dataItem != dataItems.end(); dataItem++)
  {
    if ((*dataItem)->getId() == "p5" && (*dataItem)->getName() == "execution")
    {
      hasExec = true;
    }
    
    if ((*dataItem)->getId() == "z2" && (*dataItem)->getName() == "Zcom")
    {
      hasZcom = true;
    }
  }
  
  CPPUNIT_ASSERT(hasExec);
  CPPUNIT_ASSERT(hasZcom);
}

void XmlParserTest::testCondition()
{
  CPPUNIT_ASSERT_EQUAL((size_t) 1, mDevices.size());
  
  Device *device = mDevices.front();
  list<DataItem*> dataItems;
  std::map<string, DataItem *> dataItemsMap = device->getDeviceDataItems();  
  
  DataItem *item = dataItemsMap["clc"];
  CPPUNIT_ASSERT(item);
  
  CPPUNIT_ASSERT_EQUAL((string) "clc", item->getId());
  CPPUNIT_ASSERT(item->isCondition());
}

void XmlParserTest::testGetDataItems()
{
  std::set<string> filter;
  
  a->getDataItems(filter, "//Linear");
  CPPUNIT_ASSERT_EQUAL(11, (int) filter.size());

  filter.clear();
  a->getDataItems(filter, "//Linear//DataItem[@category='CONDITION']");
  CPPUNIT_ASSERT_EQUAL(3, (int) filter.size());

  filter.clear();
  a->getDataItems(filter, "//Controller/electric/*");
  CPPUNIT_ASSERT_EQUAL(0, (int) filter.size());
  
  filter.clear();
  a->getDataItems(filter, "//Device/DataItems");
  CPPUNIT_ASSERT_EQUAL(2, (int) filter.size());  
  
  filter.clear();
  a->getDataItems(filter, "//Device/DataItems/");
  CPPUNIT_ASSERT_EQUAL(0, (int) filter.size());  
  
  filter.clear();
  a->getDataItems(filter, "//Rotary[@name=\"C\"]//DataItem[@type=\"LOAD\"]");
  CPPUNIT_ASSERT_EQUAL(2, (int) filter.size());    
}

void XmlParserTest::testGetDataItemsExt()
{
  std::set<string> filter;

  // For the rest we will check with the extended schema
  try
  {
    a = new XmlParser();
    a->parseFile("../samples/extension.xml");
  }
  catch (exception & e)
  {
    CPPUNIT_FAIL("Could not locate test xml: ../samples/extension.xml");
  }
  
  filter.clear();
  a->getDataItems(filter, "//Device//Pump");
  CPPUNIT_ASSERT_EQUAL(0, (int) filter.size());

  filter.clear();
  a->getDataItems(filter, "//Device//x:Pump");
  CPPUNIT_ASSERT_EQUAL(1, (int) filter.size());

}

void XmlParserTest::testExtendedSchema()
{
  delete a; a = NULL;
  try
  {
    a = new XmlParser();
    mDevices = a->parseFile("../samples/extension.xml");
  }
  catch (exception & e)
  {
    CPPUNIT_FAIL("Could not locate test xml: ../samples/extension.xml");
  }
  
  CPPUNIT_ASSERT_EQUAL((size_t) 1, mDevices.size());
  
  Device *device = mDevices.front();
  
  // Check for Description
  CPPUNIT_ASSERT_EQUAL((string) "Extended Schema.", device->getDescriptionBody());
  
  Component *pump = device->getChildren().front();
  CPPUNIT_ASSERT_EQUAL((string) "pump", pump->getName());
  CPPUNIT_ASSERT_EQUAL((string) "Pump", pump->getClass());
  CPPUNIT_ASSERT_EQUAL((string) "x", pump->getPrefix());
  
  DataItem *item = pump->getDataItems().front();
  CPPUNIT_ASSERT_EQUAL((string) "x:FLOW", item->getType());
  CPPUNIT_ASSERT_EQUAL((string) "Flow", item->getElementName());
  CPPUNIT_ASSERT_EQUAL((string) "x", item->getPrefix());
}

void XmlParserTest::testTimeSeries()
{
  Device *dev = mDevices[0];
  CPPUNIT_ASSERT(dev != NULL);
  
  DataItem *item = dev->getDeviceDataItem("Xact");
  CPPUNIT_ASSERT(item != NULL);
  
  item->getAttributes();
  CPPUNIT_ASSERT_EQUAL((string)"AVERAGE", item->getStatistic());
  
  std::map<std::string, std::string> *attrs = item->getAttributes();
  CPPUNIT_ASSERT_EQUAL(string("AVERAGE"), (*attrs)["statistic"]);

  
  item = dev->getDeviceDataItem("Xts");
  CPPUNIT_ASSERT(item != NULL);
  item->getAttributes();
  CPPUNIT_ASSERT(item->isTimeSeries());  
  CPPUNIT_ASSERT_EQUAL(DataItem::TIME_SERIES, item->getRepresentation());
  
  attrs = item->getAttributes();
  CPPUNIT_ASSERT_EQUAL(string("TIME_SERIES"), (*attrs)["representation"]);
}

void XmlParserTest::testConfiguration()
{
  Device *dev = mDevices[0];
  CPPUNIT_ASSERT(dev != NULL);

  Component *power = NULL;
  std::list<Component *> &children = dev->getChildren();
  std::list<Component *>::iterator iter;
  for (iter = children.begin(); power == NULL && iter != children.end(); ++iter)
  {
    if ((*iter)->getName() == "power")
      power = *iter;
  }
  
  CPPUNIT_ASSERT(!power->getConfiguration().empty());
}

void XmlParserTest::testParseAsset()
{
  string document = getFile("asset1.xml");
  AssetPtr asset = a->parseAsset("XXX", "CuttingTool", document);
  CuttingToolPtr tool = (CuttingTool*) asset.getObject();
  
  CPPUNIT_ASSERT_EQUAL((string) "KSSP300R4SD43L240", tool->mIdentity["toolId"]);
  CPPUNIT_ASSERT_EQUAL((string) "KSSP300R4SD43L240.1", tool->getAssetId());
  CPPUNIT_ASSERT_EQUAL((string) "1", tool->mIdentity["serialNumber"]);
  CPPUNIT_ASSERT_EQUAL((string) "KMT,Parlec", tool->mIdentity["manufacturers"]);
  CPPUNIT_ASSERT_EQUAL((string) "2011-05-11T13:55:22", tool->getTimestamp());
  
  // Top Level
  CPPUNIT_ASSERT_EQUAL((string) "ISO 13399...", tool->mValues["CuttingToolDefinition"]->mValue);
  CPPUNIT_ASSERT_EQUAL((string) "EXPRESS", tool->mValues["CuttingToolDefinition"]->mProperties["format"]);
  CPPUNIT_ASSERT_EQUAL((string) "Cutting tool ...", tool->mValues["Description"]->mValue);
  
  // Status
  CPPUNIT_ASSERT_EQUAL((string) "NEW", tool->mStatus[0]);
  
  // Values
  CPPUNIT_ASSERT_EQUAL((string) "10000", tool->mValues["ProgramSpindleSpeed"]->mValue);
  CPPUNIT_ASSERT_EQUAL((string) "222", tool->mValues["ProgramFeedRate"]->mValue);
  CPPUNIT_ASSERT_EQUAL((unsigned int) 1, tool->mValues["ProgramFeedRate"]->refCount());
  
  // Measurements
  CPPUNIT_ASSERT_EQUAL((string) "73.25", tool->mMeasurements["BodyDiameterMax"]->mValue);
  CPPUNIT_ASSERT_EQUAL((string) "76.2", tool->mMeasurements["CuttingDiameterMax"]->mValue);
  CPPUNIT_ASSERT_EQUAL((unsigned int) 1, tool->mMeasurements["BodyDiameterMax"]->refCount());
  
  // Items
  CPPUNIT_ASSERT_EQUAL((string) "24", tool->mItemCount);
  
  // Item
  CPPUNIT_ASSERT_EQUAL((size_t) 6, tool->mItems.size());
  CuttingItemPtr item = tool->mItems[0];
  CPPUNIT_ASSERT_EQUAL((unsigned int) 2, item->refCount());
  
  CPPUNIT_ASSERT_EQUAL((string) "SDET43PDER8GB", item->mIdentity["itemId"]);
  CPPUNIT_ASSERT_EQUAL((string) "FLANGE: 1-4, ROW: 1", item->mValues["Locus"]->mValue);
  CPPUNIT_ASSERT_EQUAL((string) "12.7", item->mMeasurements["CuttingEdgeLength"]->mValue);
  CPPUNIT_ASSERT_EQUAL((unsigned int) 1, item->mMeasurements["CuttingEdgeLength"]->refCount());
}

void XmlParserTest::testUpdateAsset()
{
  string document = getFile("asset1.xml");
  AssetPtr asset = a->parseAsset("XXX", "CuttingTool", document);
  CuttingToolPtr tool = (CuttingTool*) asset.getObject();

  string replacement = "<CuttingDiameterMax code=\"DC\" nominal=\"76.2\" maximum=\"76.213\" minimum=\"76.187\">10.123</CuttingDiameterMax>";
  a->updateAsset(asset, "CuttingTool", replacement);

  CuttingItemPtr item = tool->mItems[0];
  CPPUNIT_ASSERT_EQUAL((string) "10.123", tool->mMeasurements["CuttingDiameterMax"]->mValue);
  
  // Test cutting item replacement
  CPPUNIT_ASSERT_EQUAL((string) "12.7", item->mMeasurements["CuttingEdgeLength"]->mValue);

  replacement = "<CuttingItem indices=\"1-4\" itemId=\"SDET43PDER8GB\" manufacturers=\"KMT\" grade=\"KC725M\">"
                "<Locus>FLANGE: 1-4, ROW: 1</Locus>"
                "<Measurements>"
                "<CuttingEdgeLength code=\"L\" nominal=\"12.7\" minimum=\"12.675\" maximum=\"12.725\">14.7</CuttingEdgeLength>"
                "<WiperEdgeLength code=\"BS\" nominal=\"2.56\">2.56</WiperEdgeLength>"
                "<IncribedCircleDiameter code=\"IC\" nominal=\"12.7\">12.7</IncribedCircleDiameter>"
                "<CornerRadius code=\"RE\" nominal=\"0.8\">0.8</CornerRadius>"
                "</Measurements>"
                "</CuttingItem>";
  
  a->updateAsset(asset, "CuttingTool", replacement);

  item = tool->mItems[0];
  CPPUNIT_ASSERT_EQUAL((string) "14.7", item->mMeasurements["CuttingEdgeLength"]->mValue);
}
