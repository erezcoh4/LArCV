////////////////////////////////////////////////////////////////////////
// Class:       Supera
// Module Type: filter
// File:        Supera_module.cc
//
// Generated at Sat Apr  9 06:34:01 2016 by Kazuhiro Terao using artmod
// from cetpkgsupport v1_10_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

// nutools
#include "SimulationBase/MCTruth.h"
// larsoft
#include "larcore/Geometry/Geometry.h"
#include "larsim/Simulation/SimChannel.h"
#include "lardata/RecoBase/Wire.h"
#include "lardata/RawData/RawDigit.h"
#include "lardata/MCBase/MCTrack.h"
#include "lardata/MCBase/MCShower.h"
#include "lardata/RawData/OpDetWaveform.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusProvider.h"
// larcv
#include "SuperaCore.h"


class Supera;

class Supera : public art::EDFilter {
public:
  explicit Supera(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  Supera(Supera const &) = delete;
  Supera(Supera &&) = delete;
  Supera & operator = (Supera const &) = delete;
  Supera & operator = (Supera &&) = delete;

  // Required functions.
  bool filter(art::Event & e) override;

  void beginJob();
  void endJob();
  const ::larcv::logger& logger() const { return _logger;}

private:

  ::larcv::supera::SuperaCore< raw::OpDetWaveform, recob::Wire,
			       simb::MCTruth, sim::MCTrack, sim::MCShower, sim::SimChannel> _core;
  ::larcv::logger _logger;

};

Supera::Supera(fhicl::ParameterSet const & main_cfg)
{
  _core.configure(main_cfg);
}

void Supera::beginJob()
{
  _core.initialize();
}

void Supera::endJob()
{
  _core.finalize();
}

bool Supera::filter(art::Event & e)
{
  _core.clear_data();
  _core.set_id(e.id().run(),e.id().subRun(),e.id().event());

  art::Handle<std::vector<recob::Wire> > wire_h;
  e.getByLabel(_core.producer_wire(),wire_h);
  if(!wire_h.isValid()) { throw ::larcv::larbys("Could not load wire data!"); }

  art::Handle<std::vector<raw::OpDetWaveform> > opdigit_h;
  e.getByLabel(_core.producer_opdigit(),opdigit_h);
  if(!opdigit_h.isValid()) { throw ::larcv::larbys("Could not load opdigit data!"); }

  //
  // Fill Channel Status
  // 
  if(_core.store_chstatus()) {

    std::vector<bool> filled_ch( ::larcv::supera::Nchannels(), false );
    
    // If specified check RawDigit pedestal value: if negative this channel is not used by wire (set status=>-2)
    if(!_core.producer_digit().empty()) {
      art::Handle<std::vector<raw::RawDigit> > digit_h;
      e.getByLabel(_core.producer_digit(),digit_h);
      for(auto const& digit : *digit_h) {
	auto const ch = digit.Channel();
	if(ch >= filled_ch.size()) throw ::larcv::larbys("Found RawDigit > possible channel number!");
	if(digit.GetPedestal()<0.) {
	  _core.set_chstatus(ch,::larcv::chstatus::kNEGATIVEPEDESTAL);
	  filled_ch[ch] = true;
	}
      }
    }

    // Set database status
    const lariov::ChannelStatusProvider& chanFilt = art::ServiceHandle<lariov::ChannelStatusService>()->GetProvider();
    for(size_t i=0; i < ::larcv::supera::Nchannels(); ++i) {
      if ( filled_ch[i] ) continue;
      if (!chanFilt.IsPresent(i)) _core.set_chstatus(i,::larcv::chstatus::kNOTPRESENT);
      else _core.set_chstatus(i,(short)(chanFilt.Status(i)));
    }
  }

  bool status=true;
  if(_core.use_mc()) {

    art::Handle<std::vector<simb::MCTruth> > mctruth_h;
    art::Handle<std::vector<sim::MCTrack>  > mctrack_h;
    art::Handle<std::vector<sim::MCShower> > mcshower_h;
    e.getByLabel(_core.producer_generator(), mctruth_h);
    e.getByLabel(_core.producer_mcreco(),    mctrack_h);
    e.getByLabel(_core.producer_mcreco(),    mcshower_h);

    if(!mctruth_h.isValid() || !mctrack_h.isValid() || !mcshower_h.isValid())

      throw ::larcv::larbys("Necessary MC info missing...");

    if(_core.producer_simch().empty()) {

      std::vector<sim::SimChannel> empty_simch;
      status = _core.process_event(*opdigit_h, *wire_h, *mctruth_h, *mctrack_h, *mcshower_h, empty_simch);

    }else{

      art::Handle<std::vector<sim::SimChannel> > simch_h;
      e.getByLabel(_core.producer_simch(), simch_h);
      if(!simch_h.isValid()) throw ::larcv::larbys("SimChannel requested but not available");
      status = _core.process_event(*opdigit_h, *wire_h, *mctruth_h, *mctrack_h, *mcshower_h, *simch_h);

    }
  }else{
    std::vector<simb::MCTruth>   empty_mctruth;
    std::vector<sim::MCTrack>    empty_mctrack;
    std::vector<sim::MCShower>   empty_mcshower;
    std::vector<sim::SimChannel> empty_simch;
    status = _core.process_event(*opdigit_h, *wire_h,empty_mctruth,empty_mctrack,empty_mcshower,empty_simch);
  }
  return status;
}

DEFINE_ART_MODULE(Supera)