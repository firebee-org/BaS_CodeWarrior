library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.parameter_pkg.all;
use work.types_pkg.all;

entity tb_pipeline is generic (
	clk_period : time := 10 ns
	);

	
end entity tb_pipeline;


architecture uut of tb_pipeline is

	signal clk : std_logic := '0';
	signal rst : std_logic;

	component pipeline is port(
		clk, rst : std_logic
	);
	end component pipeline;

begin



	uut: pipeline port map(
	clk => clk,
	rst => rst
	);

	clk_gen: process
	begin
		wait for clk_period/2;
		clk <= not clk;
	end process clk_gen;

	rst_gen : process 
	begin
		rst <= '1';
		wait for 10 * clk_period;
		rst <= '0';
		wait;
	end process rst_gen;

end architecture uut;
