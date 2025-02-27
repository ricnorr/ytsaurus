#include "secondary_index.h"

#include <yt/yt/server/master/table_server/table_node.h>

#include <yt/yt/server/master/cell_master/serialize.h>

namespace NYT::NTableServer {

using namespace NCellMaster;
using namespace NTableServer;

////////////////////////////////////////////////////////////////////////////////

std::string TSecondaryIndex::GetLowercaseObjectName() const
{
    return Format("secondary index %v", GetId());
}

std::string TSecondaryIndex::GetCapitalizedObjectName() const
{
    return Format("Secondary index %v", GetId());
}

void TSecondaryIndex::Save(TSaveContext& context) const
{
    TObject::Save(context);

    using NYT::Save;
    Save(context, TableId_);
    Save(context, IndexTableId_);
    Save(context, Kind_);
    Save(context, ExternalCellTag_);
    Save(context, Predicate_);
}

void TSecondaryIndex::Load(TLoadContext& context)
{
    TObject::Load(context);

    using NYT::Load;
    // COMPAT(sabdenovch)
    if (context.GetVersion() >= EMasterReign::SecondaryIndexExternalCellTag) {
        Load(context, TableId_);
        Load(context, IndexTableId_);
    } else {
        Load<TTableNode*>(context, CompatTable_);
        Load<TTableNode*>(context, CompatIndexTable_);
    }
    Load(context, Kind_);
    // COMPAT(sabdenovch)
    if (context.GetVersion() >= EMasterReign::SecondaryIndexReplication) {
        Load(context, ExternalCellTag_);
    }
    // COMPAT(sabdenovch)
    if (context.GetVersion() >= EMasterReign::SecondaryIndexPredicate ||
        (context.GetVersion() >= EMasterReign::SecondaryIndexPredicate_24_1 &&
        context.GetVersion() < EMasterReign::DropLegacyClusterNodeMap))
    {
        Load(context, Predicate_);
    }
}

void TSecondaryIndex::SetIdsFromCompat()
{
    TableId_ = CompatTable_->GetId();
    IndexTableId_ = CompatIndexTable_->GetId();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NTableServer
