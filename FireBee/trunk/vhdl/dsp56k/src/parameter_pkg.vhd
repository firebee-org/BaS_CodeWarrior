------------------------------------------------------------------------------
--! @file
--! @author Matthias Alles
--! @date 01/2009
--! @brief Global parameters
--!
------------------------------------------------------------------------------

package parameter_pkg is

	constant BW_ADDRESS : natural := 16;

	-- number of pipeline register stages
	constant PIPELINE_DEPTH : natural := 4;

	constant NUM_ACT_SIGNALS : natural := 26;

end package;
