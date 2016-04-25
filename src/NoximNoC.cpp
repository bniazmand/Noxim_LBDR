/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the Network-on-Chip
 */

#include "NoximNoC.h"
#include <iostream>
#include <fstream>

using std::ifstream;
using std::ofstream;

void NoximNoC::buildMesh()
{
    // Check for routing table availability
    if (NoximGlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
	assert(grtable.load(NoximGlobalParams::routing_table_filename));

    // Check for traffic table availability
    if (NoximGlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
	assert(gttable.load(NoximGlobalParams::traffic_table_filename));

    // Create the mesh as a matrix of tiles
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
	for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
	    // Create the single Tile with a proper name
	    char tile_name[20];
	    sprintf(tile_name, "Tile[%02d][%02d]", i, j);
	    t[i][j] = new NoximTile(tile_name);

	    // Tell to the router its coordinates
	    t[i][j]->r->configure(j * NoximGlobalParams::mesh_dim_x + i,
				  NoximGlobalParams::stats_warm_up_time,
				  NoximGlobalParams::buffer_depth,
				  grtable);

	    // Tell to the PE its coordinates
	    t[i][j]->pe->local_id = j * NoximGlobalParams::mesh_dim_x + i;
	    t[i][j]->pe->traffic_table = &gttable;	// Needed to choose destination
	    t[i][j]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j]->pe->local_id) == 0);

	    // Map clock and reset
	    t[i][j]->clock(clock);
	    t[i][j]->reset(reset);

	    // Map Rx signals
	    t[i][j]->req_rx[DIRECTION_NORTH] (req_to_south[i][j]);
	    t[i][j]->flit_rx[DIRECTION_NORTH] (flit_to_south[i][j]);
	    t[i][j]->ack_rx[DIRECTION_NORTH] (ack_to_north[i][j]);

	    t[i][j]->req_rx[DIRECTION_EAST] (req_to_west[i + 1][j]);
	    t[i][j]->flit_rx[DIRECTION_EAST] (flit_to_west[i + 1][j]);
	    t[i][j]->ack_rx[DIRECTION_EAST] (ack_to_east[i + 1][j]);

	    t[i][j]->req_rx[DIRECTION_SOUTH] (req_to_north[i][j + 1]);
	    t[i][j]->flit_rx[DIRECTION_SOUTH] (flit_to_north[i][j + 1]);
	    t[i][j]->ack_rx[DIRECTION_SOUTH] (ack_to_south[i][j + 1]);

	    t[i][j]->req_rx[DIRECTION_WEST] (req_to_east[i][j]);
	    t[i][j]->flit_rx[DIRECTION_WEST] (flit_to_east[i][j]);
	    t[i][j]->ack_rx[DIRECTION_WEST] (ack_to_west[i][j]);

	    // Map Tx signals
	    t[i][j]->req_tx[DIRECTION_NORTH] (req_to_north[i][j]);
	    t[i][j]->flit_tx[DIRECTION_NORTH] (flit_to_north[i][j]);
	    t[i][j]->ack_tx[DIRECTION_NORTH] (ack_to_south[i][j]);

	    t[i][j]->req_tx[DIRECTION_EAST] (req_to_east[i + 1][j]);
	    t[i][j]->flit_tx[DIRECTION_EAST] (flit_to_east[i + 1][j]);
	    t[i][j]->ack_tx[DIRECTION_EAST] (ack_to_west[i + 1][j]);

	    t[i][j]->req_tx[DIRECTION_SOUTH] (req_to_south[i][j + 1]);
	    t[i][j]->flit_tx[DIRECTION_SOUTH] (flit_to_south[i][j + 1]);
	    t[i][j]->ack_tx[DIRECTION_SOUTH] (ack_to_north[i][j + 1]);

	    t[i][j]->req_tx[DIRECTION_WEST] (req_to_west[i][j]);
	    t[i][j]->flit_tx[DIRECTION_WEST] (flit_to_west[i][j]);
	    t[i][j]->ack_tx[DIRECTION_WEST] (ack_to_east[i][j]);

	    // Map buffer level signals (analogy with req_tx/rx port mapping)
	    t[i][j]->free_slots[DIRECTION_NORTH] (free_slots_to_north[i][j]);
	    t[i][j]->free_slots[DIRECTION_EAST] (free_slots_to_east[i + 1][j]);
	    t[i][j]->free_slots[DIRECTION_SOUTH] (free_slots_to_south[i][j + 1]);
	    t[i][j]->free_slots[DIRECTION_WEST] (free_slots_to_west[i][j]);

	    t[i][j]->free_slots_neighbor[DIRECTION_NORTH] (free_slots_to_south[i][j]);
	    t[i][j]->free_slots_neighbor[DIRECTION_EAST] (free_slots_to_west[i + 1][j]);
	    t[i][j]->free_slots_neighbor[DIRECTION_SOUTH] (free_slots_to_north[i][j + 1]);
	    t[i][j]->free_slots_neighbor[DIRECTION_WEST] (free_slots_to_east[i][j]);

	    // NoP 
	    t[i][j]->NoP_data_out[DIRECTION_NORTH] (NoP_data_to_north[i][j]);
	    t[i][j]->NoP_data_out[DIRECTION_EAST] (NoP_data_to_east[i + 1][j]);
	    t[i][j]->NoP_data_out[DIRECTION_SOUTH] (NoP_data_to_south[i][j + 1]);
	    t[i][j]->NoP_data_out[DIRECTION_WEST] (NoP_data_to_west[i][j]);

	    t[i][j]->NoP_data_in[DIRECTION_NORTH] (NoP_data_to_south[i][j]);
	    t[i][j]->NoP_data_in[DIRECTION_EAST] (NoP_data_to_west[i + 1][j]);
	    t[i][j]->NoP_data_in[DIRECTION_SOUTH] (NoP_data_to_north[i][j + 1]);
	    t[i][j]->NoP_data_in[DIRECTION_WEST] (NoP_data_to_east[i][j]);
	}
    }
    
    // Initliazing LBDR bits (read from input file)
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
        for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
        {
            // Initializing Connectivity bits
            t[i][j] -> r -> C_n = true;
            t[i][j] -> r -> C_e = true;
            t[i][j] -> r -> C_w = true;
            t[i][j] -> r -> C_s = true;
            
            // Initializing Routing bits
            t[i][j] -> r -> R_wn = true;
            t[i][j] -> r -> R_ws = true;
            t[i][j] -> r -> R_en = true;
            t[i][j] -> r -> R_es = true;
            t[i][j] -> r -> R_ne = true;
            t[i][j] -> r -> R_nw = true;
            t[i][j] -> r -> R_se = true;
            t[i][j] -> r -> R_sw = true;
        }
    
    // Setting connectivity bits for sides of network
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
        {
            t[i][0] -> r -> C_n = false;
            t[i][NoximGlobalParams::mesh_dim_y - 1] -> r -> C_s = false;
        }
    
    for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
    {
        t[0][j] -> r -> C_w = false;
        t[NoximGlobalParams::mesh_dim_x - 1][j] -> r -> C_e = false;
    }
    
    // Set Routing bits according to XY routing
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
        for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
        {
            t[i][j] -> r -> R_ne = false;
            t[i][j] -> r -> R_nw = false;
            t[i][j] -> r -> R_se = false;
            t[i][j] -> r -> R_sw = false;
        }
    
    // Set Routing bits according to the turn-model specified in input file
    // Order of LBDR Routing bits:
    // Rwn Rws Ren Res Rnw Rne Rsw Rse
    
    int LBDR_routing_bits_sequence[8];
    
    ifstream fin; // file pointer
    cout << "\nReading LBDR Routing bits (turn model) from input file ...\n";
    fin.open("all_2d_turn_model_LBDR_bits.txt"); // open the file for reading
    
    if (!fin.good())
    {
        cout << "\nCannot open input file!\n";
        exit(0); // exit if file not found
    }
    
    fin >> LBDR_routing_bits_sequence[0] >> LBDR_routing_bits_sequence[1] >> LBDR_routing_bits_sequence[2] >> LBDR_routing_bits_sequence[3] >> LBDR_routing_bits_sequence[4] >> LBDR_routing_bits_sequence[5] >> LBDR_routing_bits_sequence[6] >> LBDR_routing_bits_sequence[7];
    
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
        for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
        {
            t[i][j] -> r -> R_wn = LBDR_routing_bits_sequence[0];
            t[i][j] -> r -> R_ws = LBDR_routing_bits_sequence[1];
            t[i][j] -> r -> R_en = LBDR_routing_bits_sequence[2];
            t[i][j] -> r -> R_es = LBDR_routing_bits_sequence[3];
            t[i][j] -> r -> R_nw = LBDR_routing_bits_sequence[4];
            t[i][j] -> r -> R_ne = LBDR_routing_bits_sequence[5];
            t[i][j] -> r -> R_sw = LBDR_routing_bits_sequence[6];
            t[i][j] -> r -> R_se = LBDR_routing_bits_sequence[7];
        }
    
    cout << "\nLBDR Routing bits (turn model) read successfully from input file!\n\n";
    
    if (t[0][0] -> r -> R_wn == true && t[0][0] -> r -> R_ws == true && t[0][0] -> r -> R_en == true && t[0][0] -> r -> R_es == true && t[0][0] -> r -> R_nw == false && t[0][0] -> r -> R_ne == false && t[0][0] -> r -> R_sw == false && t[0][0] -> r -> R_se == false)
    {
        // XY Routing
        cout << "Turn model is : XY Routing\n\n";
    }
        
    // dummy NoximNoP_data structure
    NoximNoP_data tmp_NoP;

    tmp_NoP.sender_id = NOT_VALID;

    for (int i = 0; i < DIRECTIONS; i++) {
	tmp_NoP.channel_status_neighbor[i].free_slots = NOT_VALID;
	tmp_NoP.channel_status_neighbor[i].available = false;
    }

    // Clear signals for borderline nodes
    for (int i = 0; i <= NoximGlobalParams::mesh_dim_x; i++) {
	req_to_south[i][0] = 0;
	ack_to_north[i][0] = 0;
	req_to_north[i][NoximGlobalParams::mesh_dim_y] = 0;
	ack_to_south[i][NoximGlobalParams::mesh_dim_y] = 0;

	free_slots_to_south[i][0].write(NOT_VALID);
	free_slots_to_north[i][NoximGlobalParams::mesh_dim_y].write(NOT_VALID);

	NoP_data_to_south[i][0].write(tmp_NoP);
	NoP_data_to_north[i][NoximGlobalParams::mesh_dim_y].write(tmp_NoP);

    }

    for (int j = 0; j <= NoximGlobalParams::mesh_dim_y; j++) {
	req_to_east[0][j] = 0;
	ack_to_west[0][j] = 0;
	req_to_west[NoximGlobalParams::mesh_dim_x][j] = 0;
	ack_to_east[NoximGlobalParams::mesh_dim_x][j] = 0;

	free_slots_to_east[0][j].write(NOT_VALID);
	free_slots_to_west[NoximGlobalParams::mesh_dim_x][j].write(NOT_VALID);

	NoP_data_to_east[0][j].write(tmp_NoP);
	NoP_data_to_west[NoximGlobalParams::mesh_dim_x][j].write(tmp_NoP);

    }

    // invalidate reservation table entries for non-exhistent channels
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++) {
	t[i][0]->r->reservation_table.invalidate(DIRECTION_NORTH);
	t[i][NoximGlobalParams::mesh_dim_y - 1]->r->reservation_table.invalidate(DIRECTION_SOUTH);
    }
    for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++) {
	t[0][j]->r->reservation_table.invalidate(DIRECTION_WEST);
	t[NoximGlobalParams::mesh_dim_x - 1][j]->r->reservation_table.invalidate(DIRECTION_EAST);
    }
}

NoximTile *NoximNoC::searchNode(const int id) const
{
    for (int i = 0; i < NoximGlobalParams::mesh_dim_x; i++)
	for (int j = 0; j < NoximGlobalParams::mesh_dim_y; j++)
	    if (t[i][j]->r->local_id == id)
		return t[i][j];

    return t[0][0];
}
