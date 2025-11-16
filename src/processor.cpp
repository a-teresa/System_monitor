#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() {
  // Read current jiffies
  long active = LinuxParser::ActiveJiffies();  
  long idle   = LinuxParser::IdleJiffies();   

  // Keep previous snapshot across calls
  static long prev_active = 0;
  static long prev_idle   = 0;

  long delta_active = active - prev_active;
  long delta_idle   = idle   - prev_idle;
  long delta_total  = delta_active + delta_idle;

  // Update snapshots
  prev_active = active;
  prev_idle   = idle;

  if (delta_total <= 0) return 0.0f;
  return static_cast<float>(delta_active) / static_cast<float>(delta_total);
}