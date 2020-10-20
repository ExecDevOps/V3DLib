// Control-flow graphs (CFGs)

#include <assert.h>
#include "Target/CFG.h"
#include "Target/Syntax.h"

namespace QPULib {

// ============================================================================
// Build control-flow graph
// ============================================================================

/**
 * Build a CFG for a given instruction sequence.
 */
void buildCFG(Seq<Instr> &instrs, CFG &cfg) {
  // ----------
  // First pass
  // ----------
  //
  // 1. Each instruction is a successor of the previous
  //    instruction, unless the previous instruction
  //    is an unconditional jump or halt instruction.
  //
  // 2. Compute a mapping from labels to instruction ids.

  // Number of labels in program
  int numLabels = getFreshLabelCount();

  // Mapping from labels to instruction ids
  InstrId* labelMap = new InstrId [numLabels];

  // Initialise label mapping
  for (int i = 0; i < numLabels; i++)
    labelMap[i] = -1;

  for (int i = 0; i < instrs.size(); i++) {
    // Get instruction
    Instr instr = instrs[i];

    // Is it an unconditional jump?
    bool uncond = instr.tag == BRL && instr.BRL.cond.tag == COND_ALWAYS;

    // Is it a final instruction?
    bool end = instr.tag == END || i+1 == instrs.size();

    // Add successor
    if (! (uncond || end))
      cfg[i].insert(i+1);

    // Remember location of each label
    if (instr.tag == LAB) {
      assert(instr.label() >= 0 && instr.label() < numLabels);
      labelMap[instr.label()] = i;
    }
  }

  // -----------
  // Second pass
  // -----------
  //
  // Add a successor for each conditional jump.

  for (int i = 0; i < instrs.size(); i++) {
    Instr instr = instrs[i];
    if (instr.tag == BRL) {
      assert(labelMap[instr.BRL.label] >= 0);
      cfg[i].insert(labelMap[instr.BRL.label]);
    }
  }

  // Free memory
  delete [] labelMap;
}

}  // namespace QPULib
