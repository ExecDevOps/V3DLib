#ifndef _QPULIB_V3D_INSTR_REGISTER_H
#define _QPULIB_V3D_INSTR_REGISTER_H
#include <string>
#include "Location.h"

namespace QPULib {
namespace v3d {
namespace instr {

class Register : public Location {
public: 
	Register(const char *name, v3d_qpu_waddr waddr_val);
	Register(const char *name, v3d_qpu_waddr waddr_val, v3d_qpu_mux mux_val, bool is_dest_acc = false);

	v3d_qpu_waddr to_waddr() const override { return m_waddr_val; }
	v3d_qpu_mux to_mux() const override;

	Register l() const;
	Register ll() const;
	Register hh() const;
	Register h() const;
	Register abs() const;
	Register swp() const;

	bool is_dest_acc() const { return m_is_dest_acc; }

private:
	std::string   m_name;
	v3d_qpu_waddr m_waddr_val;
	v3d_qpu_mux   m_mux_val;
	bool          m_mux_is_set  = false;
	bool          m_is_dest_acc = false;
};


class BranchDest : public Location {
public: 
	BranchDest(const char *name, v3d_qpu_waddr dest) : m_name(name), m_dest(dest) {}

	v3d_qpu_waddr to_waddr() const override { return m_dest; }
	v3d_qpu_mux to_mux() const override;

private:
	std::string   m_name;
	v3d_qpu_waddr m_dest;
};


extern Register const r0;
extern Register const r1;
extern Register const r2;
extern Register const r3;
extern Register const r4;
extern Register const r5;
extern Register const tmua;
extern Register const tmud;

// For branch
extern BranchDest const lri;
extern Register const r_unif;

}  // instr
}  // v3d
}  // QPULib

#endif  // _QPULIB_V3D_INSTR_REGISTER_H
