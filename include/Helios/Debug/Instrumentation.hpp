#pragma once
#ifndef HELIOS_INSTRUMENTATION_HPP
#define HELIOS_INSTRUMENTATION_HPP

#include <cstdint>

namespace Helios {
    class DrawStats {
    public:
        static DrawStats& get() noexcept {
            static DrawStats instance;
            return instance;
        }

        static DrawStats& snapshot() noexcept {
            static DrawStats lastFrame;
            return lastFrame;
        }

        uint64_t drawCalls = 0;
        uint64_t vertices = 0;
        uint64_t indices = 0;
        uint64_t widgetRedraws = 0;
        uint64_t widgetRelayouts = 0;
        uint64_t widgetsSubmitted = 0;
        uint64_t commands = 0;
        uint64_t batches = 0;
        uint64_t mergedCommands = 0;
        double frameCPUms = 0.0;

        void reset() noexcept {
            drawCalls = 0;
            vertices = 0;
            indices = 0;
            widgetRedraws = 0;
            widgetRelayouts = 0;
            widgetsSubmitted = 0;
            commands = 0;
            batches = 0;
            mergedCommands = 0;
            frameCPUms = 0.0;
        }
    };
}

#define HELIOS_INSTRUMENT_DRAW_CALLS(count) do { Helios::DrawStats::get().drawCalls += (count); } while(0)
#define HELIOS_INSTRUMENT_VERTICES(count) do { Helios::DrawStats::get().vertices += (count); } while(0)
#define HELIOS_INSTRUMENT_INDICES(count) do { Helios::DrawStats::get().indices += (count); } while(0)
#define HELIOS_INSTRUMENT_WIDGET_REDRAWS(count) do { Helios::DrawStats::get().widgetRedraws += (count); } while(0)
#define HELIOS_INSTRUMENT_WIDGET_RELAYOUTS(count) do { Helios::DrawStats::get().widgetRelayouts += (count); } while(0)
#define HELIOS_INSTRUMENT_WIDGETS_SUBMITTED(count) do { Helios::DrawStats::get().widgetsSubmitted += (count); } while(0)
#define HELIOS_INSTRUMENT_COMMANDS(count) do { Helios::DrawStats::get().commands += (count); } while(0)
#define HELIOS_INSTRUMENT_BATCHES(count) do { Helios::DrawStats::get().batches += (count); } while(0)
#define HELIOS_INSTRUMENT_MERGED_COMMANDS(count) do { Helios::DrawStats::get().mergedCommands += (count); } while(0)
#define HELIOS_INSTRUMENT_FRAME_CPU_MS(ms) do { Helios::DrawStats::get().frameCPUms = (ms); } while(0)

#endif // HELIOS_INSTRUMENTATION_HPP