#include "tempest.hpp"
#include <format>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace tempest;

PYBIND11_MODULE(_core, m, py::mod_gil_not_used())
{
    m.doc() = "Python library for receiving data from Tempest Weather Station";
    py::enum_<PrecipitationType>(m, "PrecipitationType")
      .value("NONE", PrecipitationType::NONE)
      .value("RAIN", PrecipitationType::RAIN)
      .value("HAIL", PrecipitationType::HAIL)
      .value("RAIN_AND_HAIL", PrecipitationType::RAIN_AND_HAIL);

    py::class_<WindObservation>(m, "WindObservation")
      .def(py::init<>())
      .def_readwrite("lull_mph", &WindObservation::lull_mph)
      .def_readwrite("avg_mph", &WindObservation::avg_mph)
      .def_readwrite("gust_mph", &WindObservation::gust_mph)
      .def_readwrite("dir_deg", &WindObservation::dir_deg)
      .def_readwrite("sample_interval_secs", &WindObservation::sample_interval_secs)
      .def("__repr__",
           [](const WindObservation& w)
           { return std::format("<WindObservation avg_mph={}>", w.avg_mph); });

    py::class_<Observation>(m, "Observation")
      .def(py::init<>())
      .def_readwrite("firmware_revision", &Observation::firmware_revision)
      .def_readwrite("station_sn", &Observation::station_sn)
      .def_readwrite("hub_sn", &Observation::hub_sn)
      .def_readwrite("epoch_time", &Observation::epoch_time)
      .def_readwrite("wind", &Observation::wind)
      .def_readwrite("pressure_inhg", &Observation::pressure_inhg)
      .def_readwrite("air_temp_f", &Observation::air_temp_f)
      .def_readwrite("relative_humidity", &Observation::relative_humidity)
      .def_readwrite("illuminance_lux", &Observation::illuminance_lux)
      .def_readwrite("uv_index", &Observation::uv_index)
      .def_readwrite("solar_radiation_wm2", &Observation::solar_radiation_wm2)
      .def_readwrite("rain_accum_in", &Observation::rain_accum_in)
      .def_readwrite("precipitation_type", &Observation::precipitation_type)
      .def_readwrite("lightning_strike_dist_mi", &Observation::lightning_strike_dist_mi)
      .def_readwrite("lightning_strike_count", &Observation::lightning_strike_count)
      .def_readwrite("battery_volts", &Observation::battery_volts)
      .def_readwrite("report_interval", &Observation::report_interval)
      .def("__repr__",
           [](const Observation& o)
           { return std::format("<Observation sn={} temp={}>", o.station_sn, o.air_temp_f); });

    py::class_<Tempest>(m, "Tempest")
      .def(py::init<bool>())
      .def("run", &Tempest::run)
      .def("join", &Tempest::join)
      .def("add_handler", &Tempest::add_handler);
}
