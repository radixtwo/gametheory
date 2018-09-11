#!/opt/local/bin/perl
use strict;
use Graph::Easy;

my $graph = Graph::Easy->new();

$graph->add_edge('0', '1', 'o');
$graph->add_edge('0', '2', 'o');
$graph->add_edge('0', '3', 'x');
$graph->add_edge('0', '4', 'o');
$graph->add_edge('0', '5', 'o');

$graph->add_edge('1', '2', 'x');
$graph->add_edge('1', '3', 'x');
$graph->add_edge('1', '4', 'o');
$graph->add_edge('1', '5', 'x');

$graph->add_edge('2', '3', 'o');
$graph->add_edge('2', '4', 'o');
$graph->add_edge('2', '5', 'x');

$graph->add_edge('3', '4', 'x');
$graph->add_edge('3', '5', 'o');

$graph->add_edge('4', '5', 'o');

print $graph->as_ascii();
