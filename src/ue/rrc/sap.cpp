//
// This file is a part of UERANSIM open source project.
// Copyright (c) 2021 ALİ GÜNGÖR.
//
// The software and all associated files are licensed under GPL-3.0
// and subject to the terms and conditions defined in LICENSE file.
//

#include "layer.hpp"

#include <lib/rrc/encode.hpp>
#include <ue/l3/task.hpp>
#include <ue/nts.hpp>
#include <ue/rls/task.hpp>

namespace nr::ue
{

void UeRrcLayer::handleRlsSapMessage(NmUeRlsToRrc &msg)
{
    switch (msg.present)
    {
    case NmUeRlsToRrc::SIGNAL_CHANGED: {
        handleCellSignalChange(msg.cellId, msg.dbm);
        break;
    }
    case NmUeRlsToRrc::DOWNLINK_RRC_DELIVERY: {
        handleDownlinkRrc(msg.cellId, msg.channel, msg.pdu);
        break;
    }
    case NmUeRlsToRrc::RADIO_LINK_FAILURE: {
        handleRadioLinkFailure(msg.rlfCause);
        break;
    }
    }
}

void UeRrcLayer::handleNasSapMessage(NmUeNasToRrc &msg)
{
    switch (msg.present)
    {
    case NmUeNasToRrc::UPLINK_NAS_DELIVERY: {
        deliverUplinkNas(msg.pduId, std::move(msg.nasPdu));
        break;
    }
    case NmUeNasToRrc::LOCAL_RELEASE_CONNECTION: {
        // TODO: handle treat barred
        (void)msg.treatBarred;

        switchState(ERrcState::RRC_IDLE);
        m_base->rlsTask->push(std::make_unique<NmUeRrcToRls>(NmUeRrcToRls::RESET_STI));
        m_base->l3Task->push(std::make_unique<NmUeRrcToNas>(NmUeRrcToNas::RRC_CONNECTION_RELEASE));
        break;
    }
    case NmUeNasToRrc::RRC_NOTIFY: {
        triggerCycle();
        break;
    }
    }
}

} // namespace nr::ue