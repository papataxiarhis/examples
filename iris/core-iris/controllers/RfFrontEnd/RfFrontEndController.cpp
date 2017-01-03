/**
 * \file controllers/RfFrontEnd/RfFrontEndController.cpp
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * A generic RF front-end controller. This controller
 * provides a gui to manually adjust parameters including frequency,
 * bandwidth and power.
 */

#include "RfFrontEndController.h"

#include <sstream>
#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;

namespace iris
{

//! Export library functions
IRIS_CONTROLLER_EXPORTS(RfFrontEndController);

RfFrontEndController::RfFrontEndController()
  : Controller("RfFrontEnd", "A generic front-end controller",
               "Paul Sutton", "0.1")
{
  registerParameter("minfrequency", "Minimum frequency", "3498000000",
      false, minFrequency_x, Interval<double>(0,6000000000));
  registerParameter("maxfrequency", "Maximum frequency", "3512000000",
      false, maxFrequency_x, Interval<double>(0,6000000000));
  registerParameter("stepfrequency", "Frequency step size", "100000",
      false, stepFrequency_x);
  registerParameter("freqknob", "Freq Knob Used", "false",
                    false,  freqknob_used_x);

  registerParameter("minbandwidth", "Minimum bandwidth", "200000",
      false, minBandwidth_x, Interval<double>(0,20000000));
  registerParameter("maxbandwidth", "Maximum bandwidth", "5000000",
      false, maxBandwidth_x, Interval<double>(0,20000000));
  registerParameter("stepbandwidth", "Bandwidth step size", "250000",
      false, stepBandwidth_x);
  registerParameter("bwknob", "BW Knob Used", "false",
                    false,  bwknob_used_x);

  registerParameter("mingain", "Minimum gain", "0",
      false, minGain_x, Interval<double>(0,32));
  registerParameter("maxgain", "Maximum gain", "32",
      false, maxGain_x, Interval<double>(0,32));
  registerParameter("stepgain", "Gain step size", "1",
      false, stepGain_x);
  registerParameter("gainknob", "Gain Knob Used", "false",
                    false,  gainknob_used_x);

  registerParameter("minsyncmargin", "Minimum sync time safety margin", "0",
      false, min_sync_margin_tx_x, Interval<double>(0,1));
  registerParameter("maxsyncmargin", "Maximum sync time safety margin", "0.5",
      false, max_sync_margin_tx_x, Interval<double>(0,1));
  registerParameter("stepsyncmargin", "Sync time safety margin step size", "0.05",
      false, step_sync_margin_tx_x);
  registerParameter("marginknob", "Margin Knob Used", "false",
                    false,  marginknob_used_x);

  registerParameter("frontend", "Name of front end component",
                    "usrptx1", false, frontEnd_x);
  registerParameter("frontendengine", "Engine containing our front end",
                    "phyengine6", false, frontEndEngine_x);
}

void RfFrontEndController::subscribeToEvents()
{
}

void RfFrontEndController::initialize()
{
  double m = 1000000.0;
  FEGuiParams p;
  p.minFreq = minFrequency_x/m;
  p.maxFreq = maxFrequency_x/m;
  p.stepFreq = stepFrequency_x/m;
  p.knobFreq = freqknob_used_x;

  p.minBW = minBandwidth_x/m;
  p.maxBW = maxBandwidth_x/m;
  p.stepBW = stepBandwidth_x/m;
  p.knobBW = bwknob_used_x;

  p.minGain = minGain_x;
  p.maxGain = maxGain_x;
  p.stepGain = stepGain_x;
  p.knobGain = gainknob_used_x;

  p.min_sync_margin_tx = min_sync_margin_tx_x;
  p.max_sync_margin_tx = max_sync_margin_tx_x;
  p.step_sync_margin_tx = step_sync_margin_tx_x;
  p.knobMargin = marginknob_used_x;

  gui_.reset(new RfFrontEndGui(p,this));
}

void RfFrontEndController::processEvent(Event &e)
{
  if(e.eventName == "guifrequency")
    processFrequency(boost::any_cast<double>(e.data.front()));
  if(e.eventName == "guibandwidth")
    processBandwidth(boost::any_cast<double>(e.data.front()));
  if(e.eventName == "guigain")
    processGain(boost::any_cast<double>(e.data.front()));
  if(e.eventName == "guisyncmargin")
    processSyncMargin(boost::any_cast<double>(e.data.front()));
}

void RfFrontEndController::destroy()
{}

void RfFrontEndController::processFrequency(double f)
{
  LOG(LDEBUG) << "Frequency: " << f << "MHz";
  ReconfigSet r;
  ParametricReconfig p;
  p.engineName = frontEndEngine_x;
  p.componentName = frontEnd_x;
  p.parameterName = "frequency";
  stringstream str;
  str << f*1000000;
  p.parameterValue = str.str();

  r.paramReconfigs.push_back(p);
  reconfigureRadio(r);
}

void RfFrontEndController::processBandwidth(double b)
{
  LOG(LDEBUG) << "Bandwidth: " << b << "MHz";
  ReconfigSet r;
  ParametricReconfig p;
  p.engineName = frontEndEngine_x;
  p.componentName = frontEnd_x;
  p.parameterName = "rate";
  stringstream str;
  str << b*1000000;
  p.parameterValue = str.str();

  r.paramReconfigs.push_back(p);
  reconfigureRadio(r);
}

void RfFrontEndController::processGain(double g)
{
  LOG(LDEBUG) << "Gain: " << g;
  ReconfigSet r;
  ParametricReconfig p;
  p.engineName = "phyengine6"; //frontEndEngine_x;
  p.componentName = "usrptx1"; //frontEnd_x;
  p.parameterName = "gain";
  stringstream str;
  str << g;
  p.parameterValue = str.str();

  r.paramReconfigs.push_back(p);
  reconfigureRadio(r);
}

void RfFrontEndController::processSyncMargin(double m)
{
  LOG(LDEBUG) << "Sync safety margin: " << m;
  ReconfigSet r;
  ParametricReconfig p;
  p.engineName = "stackengine1";
  p.componentName = "radarmac0";
  p.parameterName = "safetytxmargin";
  stringstream str;
  str << m;
  p.parameterValue = str.str();

  r.paramReconfigs.push_back(p);
  reconfigureRadio(r);
}

} // namespace iris