------------------------------------------------------------------------------
--! @file
--! @author Matthias Alles
--! @date 01/2009
--! @brief Address generation logic
--!
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.parameter_pkg.all;
use work.types_pkg.all;
use work.constants_pkg.all;

entity adgen_stage is port(
	activate_adgen    : in  std_logic;
	activate_x_mem    : in  std_logic;
	activate_y_mem    : in  std_logic;
	activate_l_mem    : in  std_logic;
	instr_word        : in  std_logic_vector(23 downto 0);
	instr_array       : in  instructions_type;
	optional_ea_word  : in  std_logic_vector(23 downto 0);
	register_file     : in  register_file_type;
	adgen_mode_a      : in  adgen_mode_type;
	adgen_mode_b      : in  adgen_mode_type;
	address_out_x     : out unsigned(BW_ADDRESS-1 downto 0);
	address_out_y     : out unsigned(BW_ADDRESS-1 downto 0);
	wr_R_port_A_valid : out std_logic;
	wr_R_port_A       : out addr_wr_port_type;
	wr_R_port_B_valid : out std_logic;
	wr_R_port_B       : out addr_wr_port_type
);
end entity;


architecture rtl of adgen_stage is

	signal address_out_x_int : unsigned(BW_ADDRESS-1 downto 0);

	signal r_reg_local_x : unsigned(BW_ADDRESS-1 downto 0);
	signal n_reg_local_x : unsigned(BW_ADDRESS-1 downto 0);
	signal m_reg_local_x : unsigned(BW_ADDRESS-1 downto 0);

	signal r_reg_local_y : unsigned(BW_ADDRESS-1 downto 0);
	signal n_reg_local_y : unsigned(BW_ADDRESS-1 downto 0);
	signal m_reg_local_y : unsigned(BW_ADDRESS-1 downto 0);

	function calculate_modulo_bitmask(m_reg_local : in unsigned ) return std_logic_vector is
		variable modulo_bitmask_intern : std_logic_vector(BW_ADDRESS-1 downto 0);
	begin
		modulo_bitmask_intern(BW_ADDRESS-1) := m_reg_local(BW_ADDRESS-1);
		for i in BW_ADDRESS-2 downto 0 loop
			modulo_bitmask_intern(i) := modulo_bitmask_intern(i+1) or m_reg_local(i);
		end loop;

		return modulo_bitmask_intern;
	end function calculate_modulo_bitmask;

	function calculate_new_r_reg(new_r_reg_intermediate, r_reg_local, m_reg_local: in unsigned;
	                             modulo_bitmask: in std_logic_vector ) return unsigned is
		variable modulo_result : unsigned(BW_ADDRESS-1 downto 0);
		variable new_r_reg_intern : unsigned(BW_ADDRESS-1 downto 0);
	begin
		-- cut out the bits we are interested in
		-- for modulo addressing
		for i in 0 to BW_ADDRESS-1 loop
			if modulo_bitmask(i) = '1' then
				modulo_result(i) := new_r_reg_intermediate(i);
			else
				modulo_result(i) := '0';
			end if;
		end loop;
		-- compare whether an overflow occurred and we
		-- have to renormalize the result
		if modulo_result > m_reg_local then
			modulo_result := modulo_result - m_reg_local;
		end if;

		-- linear addressing
		if m_reg_local = 2**BW_ADDRESS-1 then 
			new_r_reg_intern := new_r_reg_intermediate;
		-- bit reverse operation
		elsif m_reg_local = 0 then
			for i in 0 to BW_ADDRESS-1 loop
				new_r_reg_intern(BW_ADDRESS - 1 - i) := new_r_reg_intermediate(i);
			end loop;
		-- modulo arithmetic / linear addressing
		else
			-- only update the bits that are part of the bitmask!
			for i in 0 to BW_ADDRESS-1 loop
				if modulo_bitmask(i) = '1' then
					new_r_reg_intern(i) := modulo_result(i);
				else
					new_r_reg_intern(i) := r_reg_local(i);
				end if;
			end loop;
		end if;
		return new_r_reg_intern;
	end function calculate_new_r_reg;

	procedure set_operands(r_reg_local, m_reg_local, addr_mod : in unsigned; op1, op2 : out unsigned) is
	begin
		-- bit reverse operation
		if m_reg_local = 0 then
			-- reverse the input to the adder bit wise
			-- so we just need to use a single adder
			for i in 0 to BW_ADDRESS-1 loop
				op1(BW_ADDRESS - 1 - i) := r_reg_local(i);
				op2(BW_ADDRESS - 1 - i) := addr_mod(i);
			end loop;
		-- modulo arithmetic / linear addressing
		else
			op1 := r_reg_local;
			op2 := addr_mod;
		end if;
	end procedure set_operands;

begin

	address_out_x <= address_out_x_int;

	r_reg_local_x <= register_file.addr_r(to_integer(unsigned(instr_word(10 downto 8))));
	n_reg_local_x <= register_file.addr_n(to_integer(unsigned(instr_word(10 downto 8))));
	m_reg_local_x <= register_file.addr_m(to_integer(unsigned(instr_word(10 downto 8))));

	r_reg_local_y <= register_file.addr_r(to_integer(unsigned((not instr_word(10)) & instr_word(14 downto 13))));
	n_reg_local_y <= register_file.addr_n(to_integer(unsigned((not instr_word(10)) & instr_word(14 downto 13))));
	m_reg_local_y <= register_file.addr_m(to_integer(unsigned((not instr_word(10)) & instr_word(14 downto 13))));

	address_generator_X: process(activate_adgen, instr_word, adgen_mode_a, r_reg_local_x, n_reg_local_x, m_reg_local_x) is
		variable op1         : unsigned(BW_ADDRESS-1 downto 0);
		variable op2         : unsigned(BW_ADDRESS-1 downto 0);
		variable addr_mod    : unsigned(BW_ADDRESS-1 downto 0);
		variable new_r_reg   : unsigned(BW_ADDRESS-1 downto 0);
		variable new_r_reg_interm : unsigned(BW_ADDRESS-1 downto 0);
		variable modulo_bitmask : std_logic_vector(BW_ADDRESS-1 downto 0);
		variable modulo_result : unsigned(BW_ADDRESS-1 downto 0);
	begin

		-- select the operands for the calculation
		case adgen_mode_a is
			-- (Rn) - Nn
			when POST_MIN_N =>  addr_mod := unsigned(- signed(n_reg_local_x));
			-- (Rn) + Nn
			when POST_PLUS_N => addr_mod :=   n_reg_local_x;
			-- (Rn)-
			when POST_MIN_1 =>  addr_mod := (others => '1'); -- -1
			-- (Rn)+
			when POST_PLUS_1 => addr_mod := to_unsigned(1, BW_ADDRESS);
			-- (Rn)
			when NOP =>         addr_mod := (others => '0');
			-- (Rn + Nn)
			when INDEXED_N =>   addr_mod := n_reg_local_x;
			-- -(Rn)
			when PRE_MIN_1 =>   addr_mod := (others => '1'); -- - 1
			-- absolute address (appended to instruction word)
			when ABSOLUTE =>    addr_mod := (others => '0');
			when IMMEDIATE =>   addr_mod := (others => '0');
		end case;

		------------------------------------------------
		-- set op1 and op2 according to modulo register
		------------------------------------------------
		set_operands(r_reg_local_x, m_reg_local_x, addr_mod, op1, op2);

		-------------------------
		-- Calculate new address
		-------------------------
		new_r_reg_interm := op1 + op2;

		----------------------------------
		-- Calculate new register content
		-----------------------------------
		modulo_bitmask := calculate_modulo_bitmask(m_reg_local_x);
		new_r_reg := calculate_new_r_reg(new_r_reg_interm, r_reg_local_x, m_reg_local_x, modulo_bitmask);

		-- store the updated register in the global register file
		-- do not store when we do nothing or there is nothing to update
		-- LUA instructions DO NOT UPDATE the source register!!
		if (adgen_mode_a = NOP or adgen_mode_a = ABSOLUTE or adgen_mode_a = IMMEDIATE or instr_array = INSTR_LUA) then
			wr_R_port_A_valid <= '0';
		else
			wr_R_port_A_valid <= '1';
		end if;
		wr_R_port_A.reg_number <= unsigned(instr_word(10 downto 8));
		wr_R_port_A.reg_value  <= new_r_reg;

		-- select the output of the AGU
		case adgen_mode_a is
			-- (Rn) - Nn
			when POST_MIN_N =>  address_out_x_int <= r_reg_local_x;
			-- (Rn) + Nn
			when POST_PLUS_N => address_out_x_int <= r_reg_local_x;
			-- (Rn)-
			when POST_MIN_1 =>  address_out_x_int <= r_reg_local_x;
			-- (Rn)+
			when POST_PLUS_1 => address_out_x_int <= r_reg_local_x;
			-- (Rn)
			when NOP =>         address_out_x_int <= r_reg_local_x;
			-- (Rn + Nn)
			when INDEXED_N =>   address_out_x_int <= new_r_reg;
			-- -(Rn)
			when PRE_MIN_1 =>   address_out_x_int <= new_r_reg;
			-- absolute address (appended to instruction word)
			when ABSOLUTE =>    address_out_x_int <= unsigned(optional_ea_word(BW_ADDRESS-1 downto 0));
			when IMMEDIATE =>   address_out_x_int <= r_reg_local_x; -- Done externally, value never used
		end case;
		-- LUA instructions only use the updated address!
		if instr_array = INSTR_LUA then
			address_out_x_int <= new_r_reg;
		end if;

	end process address_generator_X;


	---------------------------------------------------------
	-- Second address generator
	-- Used when accessing X and Y memory at the same time
	---------------------------------------------------------
	address_generator_Y: process(activate_adgen, activate_x_mem, activate_y_mem, activate_l_mem, instr_word,
	                             register_file, adgen_mode_b, address_out_x_int, r_reg_local_y, n_reg_local_y, m_reg_local_y) is
		variable op1         : unsigned(BW_ADDRESS-1 downto 0);
		variable op2         : unsigned(BW_ADDRESS-1 downto 0);
		variable addr_mod    : unsigned(BW_ADDRESS-1 downto 0);
		variable new_r_reg   : unsigned(BW_ADDRESS-1 downto 0);
		variable new_r_reg_interm : unsigned(BW_ADDRESS-1 downto 0);
		variable modulo_bitmask : std_logic_vector(BW_ADDRESS-1 downto 0);
		variable modulo_result : unsigned(BW_ADDRESS-1 downto 0);
	begin

		-- select the operands for the calculation
		case adgen_mode_b is
			-- (Rn) + Nn
			when POST_PLUS_N => addr_mod :=   n_reg_local_y;
			-- (Rn)-
			when POST_MIN_1  => addr_mod := (others => '1'); -- -1
			-- (Rn)+
			when POST_PLUS_1 => addr_mod := to_unsigned(1, BW_ADDRESS);
			-- (Rn)
			when others      => addr_mod := (others => '0');
		end case;

		------------------------------------------------
		-- set op1 and op2 according to modulo register
		------------------------------------------------
		set_operands(r_reg_local_y, m_reg_local_y, addr_mod, op1, op2);

		-------------------------
		-- Calculate new address
		-------------------------
		new_r_reg_interm := op1 + op2;

		----------------------------------
		-- Calculate new register content
		-----------------------------------
		modulo_bitmask := calculate_modulo_bitmask(m_reg_local_y);
		new_r_reg := calculate_new_r_reg(new_r_reg_interm, r_reg_local_y, m_reg_local_y, modulo_bitmask);

		-- store the updated register in the global register file
		-- do not store when we do nothing or there is nothing to update
		if adgen_mode_b = NOP then
			wr_R_port_B_valid <= '0';
		else
			wr_R_port_B_valid <= '1';
		end if;
		wr_R_port_B.reg_number <= unsigned((not instr_word(10)) & instr_word(14 downto 13));
		wr_R_port_B.reg_value  <= new_r_reg;

		-- the address for the y memory is calculated in the first AGU if the x memory is not accessed!
		-- so use the other output as address output for the y memory!
		-- Furthermore, use the same address for L memory accesses (X and Y memory access the same address!)
		if (activate_y_mem = '1' and activate_x_mem = '0') or activate_l_mem = '1' then
			address_out_y <= address_out_x_int;
		-- in any other case use the locally computed value
		else
			-- select the output of the AGU
			case adgen_mode_b is
				-- (Rn) + Nn
				when POST_PLUS_N => address_out_y <= r_reg_local_y;
				-- (Rn)-
				when POST_MIN_1  => address_out_y <= r_reg_local_y;
				-- (Rn)+
				when POST_PLUS_1 => address_out_y <= r_reg_local_y;
				-- (Rn)
				when others      => address_out_y <= r_reg_local_y;
			end case;
		end if;
	end process address_generator_Y;

end architecture;
