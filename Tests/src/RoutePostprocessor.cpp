/*
  RoutePostprocessor - a test program for libosmscout
  Copyright (C) 2024  Lukas Karas

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <cstdlib>
#include <iostream>

#include <osmscout/routing/RoutePostprocessor.h>

#include <TestMain.h>

using namespace std;
using namespace osmscout;

class MockDatabase {
private:
  TypeConfigRef typeConfig;
  FileScanner wayScanner;

public:
  MockDatabase(TypeConfigRef typeConfig, const std::string &wayFile): typeConfig(typeConfig)
  {
    wayScanner.Open(wayFile, FileScanner::Mode::FastRandom, true);
  }

  ~MockDatabase()
  {
    wayScanner.Close();
  };

  osmscout::WayRef GetWay(const osmscout::DBFileOffset &offset)
  {
    assert(offset.database==0);
    auto way=std::make_shared<Way>();
    wayScanner.SetPos(offset.offset);
    way->Read(*typeConfig, wayScanner);
    return way;
  }

  TypeConfigRef GetTypeConfig() {
    return typeConfig;
  }
};

using MockDatabaseRef = std::shared_ptr<MockDatabase>;

class MockDatabaseBuilder {
private:
  TypeConfigRef typeConfig=std::make_shared<TypeConfig>();
  FileWriter writer;
  TypeInfoRef type;
  std::string wayFile="/tmp/test-ways.dat"; // TODO: make temporary, make sure that it is deleted
  // needs to correspond to feature registration order
  static constexpr size_t laneFeatureIndex=0;
  static constexpr size_t accessFeatureIndex=1;

public:
  MockDatabaseBuilder()
  {
    osmscout::FeatureRef laneFeature = typeConfig->GetFeature(LanesFeature::NAME);
    assert(laneFeature);
    osmscout::FeatureRef accessFeature = typeConfig->GetFeature(AccessFeature::NAME);
    assert(accessFeature);

    type=std::make_shared<TypeInfo>("highway");
    // type->SetInternal()
    type->CanBeWay(true)
      .CanRouteCar(true)
      .AddFeature(laneFeature)
      .AddFeature(accessFeature);

    typeConfig->RegisterType(type);

    writer.Open(wayFile);
  }

  ~MockDatabaseBuilder()
  {
    if (writer.IsOpen()) {
      writer.Close();
    }
  }

  ObjectFileRef AddWay(const std::vector<GeoCoord> &coords, std::function<void(AccessFeatureValue*, LanesFeatureValue*)> setupFeatures)
  {
    Way way;

    FeatureValueBuffer featureValueBuffer;
    featureValueBuffer.SetType(type);

    setupFeatures(static_cast<AccessFeatureValue *>(featureValueBuffer.AllocateValue(accessFeatureIndex)),
                  static_cast<LanesFeatureValue *>(featureValueBuffer.AllocateValue(laneFeatureIndex)));

    way.SetFeatures(featureValueBuffer);

    for (const GeoCoord &coord : coords) {
      way.nodes.push_back(Point(0, coord));
    }
    ObjectFileRef wayRef(writer.GetPos(), RefType::refWay);
    way.Write(*typeConfig, writer);
    return wayRef;
  }

  MockDatabaseRef Build()
  {
    writer.Close();
    return std::make_shared<MockDatabase>(typeConfig, wayFile);
  }
};

class MockContext: public osmscout::PostprocessorContext
{
private:
  MockDatabaseRef database;
  LanesFeatureValueReader lanesReader;
  AccessFeatureValueReader accessReader;

public:
  explicit MockContext(MockDatabaseRef database):
    database(database),
    lanesReader(*database->GetTypeConfig()),
    accessReader(*database->GetTypeConfig())
  {}

  osmscout::AreaRef GetArea([[maybe_unused]] const osmscout::DBFileOffset &offset) const override
  {
    assert(false);
    return osmscout::AreaRef();
  }

  osmscout::WayRef GetWay(const osmscout::DBFileOffset &offset) const override
  {
    return database->GetWay(offset);
  }

  osmscout::NodeRef GetNode([[maybe_unused]] const osmscout::DBFileOffset &offset) const override
  {
    assert(false);
    return osmscout::NodeRef();
  }

  osmscout::Duration
  GetTime([[maybe_unused]] osmscout::DatabaseId dbId, [[maybe_unused]] const osmscout::Area &area, [[maybe_unused]] const osmscout::Distance &deltaDistance) const override
  {
    assert(false);
    return osmscout::Duration();
  }

  osmscout::Duration
  GetTime([[maybe_unused]] osmscout::DatabaseId dbId, [[maybe_unused]] const osmscout::Way &way, [[maybe_unused]] const osmscout::Distance &deltaDistance) const override
  {
    assert(false);
    return osmscout::Duration();
  }

  osmscout::RouteDescription::NameDescriptionRef
  GetNameDescription([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return osmscout::RouteDescription::NameDescriptionRef();
  }

  osmscout::RouteDescription::NameDescriptionRef
  GetNameDescription([[maybe_unused]] osmscout::DatabaseId dbId,
                     [[maybe_unused]] const osmscout::ObjectFileRef &object) const override
  {
    assert(false);
    return osmscout::RouteDescription::NameDescriptionRef();
  }

  osmscout::RouteDescription::NameDescriptionRef
  GetNameDescription([[maybe_unused]] osmscout::DatabaseId dbId,
                     [[maybe_unused]] const osmscout::Node &node) const override
  {
    assert(false);
    return osmscout::RouteDescription::NameDescriptionRef();
  }

  osmscout::RouteDescription::NameDescriptionRef
  GetNameDescription([[maybe_unused]] osmscout::DatabaseId dbId,
                     [[maybe_unused]] const osmscout::Area &area) const override
  {
    assert(false);
    return osmscout::RouteDescription::NameDescriptionRef();
  }

  osmscout::RouteDescription::NameDescriptionRef
  GetNameDescription([[maybe_unused]] osmscout::DatabaseId dbId,
                     [[maybe_unused]] const osmscout::Way &way) const override
  {
    assert(false);
    return osmscout::RouteDescription::NameDescriptionRef();
  }

  bool IsMotorwayLink([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return false;
  }

  bool IsMotorway([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return false;
  }

  bool IsMiniRoundabout([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return false;
  }

  bool IsClockwise([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return false;
  }

  bool IsRoundabout([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return false;
  }

  bool IsBridge([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return false;
  }

  osmscout::NodeRef GetJunctionNode([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return osmscout::NodeRef();
  }

  osmscout::RouteDescription::DestinationDescriptionRef
  GetDestination([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return osmscout::RouteDescription::DestinationDescriptionRef();
  }

  uint8_t GetMaxSpeed([[maybe_unused]] const osmscout::RouteDescription::Node &node) const override
  {
    assert(false);
    return 0;
  }

  osmscout::RouteDescription::LaneDescription
  GetLanes(const osmscout::DatabaseId &dbId, const osmscout::WayRef &way, bool forward) const override
  {
    assert(dbId==0);

    AccessFeatureValue *accessValue=accessReader.GetValue(way->GetFeatureValueBuffer());
    bool oneway=accessValue!=nullptr && accessValue->IsOneway();

    uint8_t laneCount;
    std::vector<LaneTurn> laneTurns;
    LanesFeatureValue *lanesValue=lanesReader.GetValue(way->GetFeatureValueBuffer());
    if (lanesValue!=nullptr) {
      laneCount=std::max((uint8_t)1,forward ? lanesValue->GetForwardLanes() : lanesValue->GetBackwardLanes());
      laneTurns=forward ? lanesValue->GetTurnForward() : lanesValue->GetTurnBackward();
      while (laneTurns.size() < laneCount) {
        laneTurns.push_back(LaneTurn::None);
      }
    } else {
      // default lane count by object type
      if (oneway) {
        laneCount=way->GetType()->GetOnewayLanes();
      } else {
        laneCount=std::max(1,way->GetType()->GetLanes()/2);
      }
    }

    return RouteDescription::LaneDescription(oneway, laneCount, laneTurns);
  }

  osmscout::RouteDescription::LaneDescriptionRef GetLanes(const osmscout::RouteDescription::Node &node) const override
  {
    RouteDescription::LaneDescriptionRef lanes;
    if (node.GetPathObject().GetType()==refWay) {

      WayRef way=GetWay(node.GetDBFileOffset());
      bool forward = node.GetCurrentNodeIndex() < node.GetTargetNodeIndex();

      lanes=std::make_shared<RouteDescription::LaneDescription>(GetLanes(node.GetDatabaseId(), way, forward));
    }
    return lanes;
  }

  osmscout::Id GetNodeId(const osmscout::RouteDescription::Node &node) const override
  {
    const ObjectFileRef& object=node.GetPathObject();
    size_t nodeIndex=node.GetCurrentNodeIndex();
    if (object.GetType()==refArea) {
      AreaRef area=GetArea(node.GetDBFileOffset());

      return area->rings.front().nodes[nodeIndex].GetId();
    }

    if (object.GetType()==refWay) {
      WayRef way=GetWay(node.GetDBFileOffset());

      return way->GetId(nodeIndex);
    }

    assert(false);
    return 0;
  }

  size_t GetNodeIndex([[maybe_unused]] const osmscout::RouteDescription::Node &node,
                      [[maybe_unused]] osmscout::Id nodeId) const override
  {
    assert(false);
    return 0;
  }

  bool CanUseBackward(const osmscout::DatabaseId &dbId, osmscout::Id fromNodeId,
                      const osmscout::ObjectFileRef &object) const override
  {
    if (object.GetType()==refWay) {
      WayRef way=GetWay(DBFileOffset(dbId,object.GetFileOffset()));

      size_t fromNodeIndex;

      if (!way->GetNodeIndexByNodeId(fromNodeId,
                                     fromNodeIndex)) {
        assert(false);
      }

      if (fromNodeIndex>0) {
        return false;
      }

      AccessFeatureValue *accessValue=accessReader.GetValue(way->GetFeatureValueBuffer());
      if (!accessValue){
        return true;
      }
      return accessValue->CanRouteCarBackward();
    }

    assert(false);
    return false;
  }

  bool CanUseForward(const osmscout::DatabaseId &dbId, osmscout::Id fromNodeId,
                     const osmscout::ObjectFileRef &object) const override
  {
    if (object.GetType()==refWay) {
      WayRef way=GetWay(DBFileOffset(dbId,
                                     object.GetFileOffset()));

      size_t fromNodeIndex;

      if (!way->GetNodeIndexByNodeId(fromNodeId,
                                     fromNodeIndex)) {
        assert(false);
      }

      if (fromNodeIndex==way->nodes.size()-1) {
        return false;
      }
      AccessFeatureValue *accessValue=accessReader.GetValue(way->GetFeatureValueBuffer());
      if (!accessValue){
        return true;
      }
      return accessValue->CanRouteCarForward();
    }
    assert(false);
    return false;
  }

  bool IsBackwardPath([[maybe_unused]] const osmscout::ObjectFileRef &object,
                      [[maybe_unused]] size_t fromNodeIndex,
                      [[maybe_unused]] size_t toNodeIndex) const override
  {
    assert(false);
    return false;
  }

  bool IsForwardPath([[maybe_unused]] const osmscout::ObjectFileRef &object,
                     [[maybe_unused]] size_t fromNodeIndex,
                     [[maybe_unused]] size_t toNodeIndex) const override
  {
    assert(false);
    return false;
  }

  bool IsNodeStartOrEndOfObject([[maybe_unused]] const osmscout::RouteDescription::Node &node,
                                [[maybe_unused]] const osmscout::ObjectFileRef &object) const override
  {
    assert(false);
    return false;
  }

  osmscout::GeoCoord GetCoordinates([[maybe_unused]] const osmscout::RouteDescription::Node &node,
                                    [[maybe_unused]] size_t nodeIndex) const override
  {
    assert(false);
    return osmscout::GeoCoord();
  }

  vector<osmscout::DatabaseRef> GetDatabases() const override
  {
    assert(false);
    return vector<osmscout::DatabaseRef>();
  }
};

TEST_CASE("Suggest right lane on simple junction")
{
  using namespace osmscout;

  // https://www.openstreetmap.org/#map=19/50.09267/14.53044
  MockDatabaseBuilder databaseBuilder;
  // street Českobrodská to east https://www.openstreetmap.org/way/936270761
  // with two lanes
  ObjectFileRef way1Ref=databaseBuilder.AddWay(
    {GeoCoord(50.09294, 14.52899), GeoCoord(50.09282, 14.52976)},
    [](AccessFeatureValue* access, LanesFeatureValue* lanes){
      lanes->SetLanes(2, 0);
      access->SetAccess(AccessFeatureValue::onewayForward | AccessFeatureValue::carForward);
    });

  // continuation of Českobrodská https://www.openstreetmap.org/way/62143601
  // single lane
  ObjectFileRef way2Ref=databaseBuilder.AddWay(
    {GeoCoord(50.09282, 14.52976), GeoCoord(50.09263, 14.53118)},
    [](AccessFeatureValue* access, LanesFeatureValue* lanes){
      lanes->SetLanes(1, 0);
      access->SetAccess(AccessFeatureValue::onewayForward | AccessFeatureValue::carForward);
    });

  // link between Českobrodská a Průmyslová https://www.openstreetmap.org/way/936270757
  // single lane
  ObjectFileRef way3Ref=databaseBuilder.AddWay(
    {GeoCoord(50.09282, 14.52976), GeoCoord(50.09261, 14.53093)},
    [](AccessFeatureValue* access, LanesFeatureValue* lanes){
      lanes->SetLanes(1, 0);
      access->SetAccess(AccessFeatureValue::onewayForward | AccessFeatureValue::carForward);
    });

  RouteDescription description;

  // route is going in direction to Průmyslová
  description.AddNode(0, 0, {way1Ref}, way1Ref, 1);
  description.AddNode(0, 0, {way1Ref, way2Ref, way3Ref}, way3Ref, 1);
  description.AddNode(0, 1, {way3Ref}, ObjectFileRef(), 0);

  MockContext context(databaseBuilder.Build());

  RoutePostprocessor::LanesPostprocessor lanesPostprocessor;
  lanesPostprocessor.Process(context, description);

  RoutePostprocessor::SuggestedLanesPostprocessor postprocessor;
  postprocessor.Process(context, description);

  // should suggest right lane
  {
    auto nodeIt = description.Nodes().begin();
    auto suggestedLanes = std::dynamic_pointer_cast<RouteDescription::SuggestedLaneDescription>(
      nodeIt->GetDescription(RouteDescription::SUGGESTED_LANES_DESC));
    REQUIRE(suggestedLanes->GetFrom() == 1);
    REQUIRE(suggestedLanes->GetTo() == 1);
  }

  description.Clear();

  // route continue to the east
  description.AddNode(0, 0, {way1Ref}, way1Ref, 1);
  description.AddNode(0, 0, {way1Ref, way2Ref, way3Ref}, way2Ref, 1);
  description.AddNode(0, 1, {way2Ref}, ObjectFileRef(), 0);

  lanesPostprocessor.Process(context, description);
  postprocessor.Process(context, description);

  // should suggest left lane
  {
    auto nodeIt = description.Nodes().begin();
    auto suggestedLanes = std::dynamic_pointer_cast<RouteDescription::SuggestedLaneDescription>(
      nodeIt->GetDescription(RouteDescription::SUGGESTED_LANES_DESC));
    REQUIRE(suggestedLanes->GetFrom() == 0);
    REQUIRE(suggestedLanes->GetTo() == 0);
  }
}