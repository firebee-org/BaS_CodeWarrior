------------------------------------------------------------------------------
--! @file
--! @author Matthias Alles
--! @date 01/2009
--! @brief Top entity of DSP
--!
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.parameter_pkg.all;
use work.types_pkg.all;
use work.constants_pkg.all;

entity dsp56k is port (
	clk, rst : in std_logic;
	-- put register file here for synthesis!
	register_file : out register_file_type
--	port_a_in     : in  port_a_in_type;
--	port_a_out    : out port_a_out_type;
--	port_b_in     : in  port_b_in_type;
--	port_b_out    : out port_b_out_type;
--	port_c_in     : in  port_c_in_type;
--	port_c_out    : out port_c_out_type;

);
end dsp56k;


architecture rtl of dsp56k is

	component pipeline is port (
		clk, rst : in std_logic;
		register_file_out : out register_file_type;
		stall_flags_out   : out std_logic_vector(PIPELINE_DEPTH-1 downto 0);
		memory_stall   : in  std_logic;
		data_rom_enable: out std_logic;
		pmem_ctrl_in   : out mem_ctrl_type_in;
		pmem_ctrl_out  : in  mem_ctrl_type_out;
		pmem2_ctrl_in  : out mem_ctrl_type_in;
		pmem2_ctrl_out : in  mem_ctrl_type_out;
		xmem_ctrl_in   : out mem_ctrl_type_in;
		xmem_ctrl_out  : in  mem_ctrl_type_out;
		ymem_ctrl_in   : out mem_ctrl_type_in;
		ymem_ctrl_out  : in  mem_ctrl_type_out
	
	);
	end component pipeline;

	component memory_management is port (
		clk, rst : in std_logic;
		stall_flags    : in  std_logic_vector(PIPELINE_DEPTH-1 downto 0);
		memory_stall   : out std_logic;
		data_rom_enable: in  std_logic;
		pmem_ctrl_in   : in  mem_ctrl_type_in;
		pmem_ctrl_out  : out mem_ctrl_type_out;
		pmem2_ctrl_in  : in  mem_ctrl_type_in;
		pmem2_ctrl_out : out mem_ctrl_type_out;
		xmem_ctrl_in   : in  mem_ctrl_type_in;
		xmem_ctrl_out  : out mem_ctrl_type_out;
		ymem_ctrl_in   : in  mem_ctrl_type_in;
		ymem_ctrl_out  : out mem_ctrl_type_out
	);
	end component memory_management;

	signal stall_flags     : std_logic_vector(PIPELINE_DEPTH-1 downto 0);
	signal memory_stall    : std_logic;
	signal data_rom_enable : std_logic;
	signal pmem_ctrl_in   : mem_ctrl_type_in;
	signal pmem_ctrl_out  : mem_ctrl_type_out;
	signal pmem2_ctrl_in  : mem_ctrl_type_in;
	signal pmem2_ctrl_out : mem_ctrl_type_out;
	signal xmem_ctrl_in   : mem_ctrl_type_in;
	signal xmem_ctrl_out  : mem_ctrl_type_out;
	signal ymem_ctrl_in   : mem_ctrl_type_in;
	signal ymem_ctrl_out  : mem_ctrl_type_out;

begin

	pipeline_inst : pipeline port map(
		clk => clk,
		rst => rst,
		register_file_out => register_file,
		stall_flags_out   => stall_flags,
		memory_stall  => memory_stall,
		data_rom_enable => data_rom_enable,
		pmem_ctrl_in   => pmem_ctrl_in,
		pmem_ctrl_out  => pmem_ctrl_out,
		pmem2_ctrl_in  => pmem2_ctrl_in,
		pmem2_ctrl_out => pmem2_ctrl_out,
		xmem_ctrl_in   => xmem_ctrl_in,
		xmem_ctrl_out  => xmem_ctrl_out,
		ymem_ctrl_in   => ymem_ctrl_in,
		ymem_ctrl_out  => ymem_ctrl_out
	);

	---------------------
	-- MEMORY MANAGEMENT
	---------------------
	MMU_inst: memory_management port map (
		clk => clk,
		rst => rst,
		stall_flags   => stall_flags,
		memory_stall  => memory_stall,
		data_rom_enable => data_rom_enable,
		pmem_ctrl_in   => pmem_ctrl_in,
		pmem_ctrl_out  => pmem_ctrl_out,
		pmem2_ctrl_in  => pmem2_ctrl_in,
		pmem2_ctrl_out => pmem2_ctrl_out,
		xmem_ctrl_in   => xmem_ctrl_in,
		xmem_ctrl_out  => xmem_ctrl_out,
		ymem_ctrl_in   => ymem_ctrl_in,
		ymem_ctrl_out  => ymem_ctrl_out
	);

end architecture rtl;
