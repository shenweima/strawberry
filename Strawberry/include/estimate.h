/*
 * qp.h
 *
 *  Created on: Oct 15, 2015
 *      Author: ruolin
 */

#ifndef QP_H_
#define QP_H_
#include <contig.h>
#include <read.h>

class Isoform{

public:
   Contig _contig;
   vector<GenomicFeature> _exon_segs;
   int _gene_id;
   int _isoform_id;
   double _bais_factor;
   string _TPM;
   string _FPKM;
   //Isoform() = default;
   Isoform(Contig contig, int gene, int isoform, vector<GenomicFeature> feats);

};

class ExonBin{
   /*
    * ExonBin is a set a continuous exons defined by an overlapping fragment.
    */
private:
   //uint _left_most;
   //uint _right_most;
   //RefID _ref_id;

   float _whole_read_mass = 0;
   /*  exon boundaries left1,right1,left2,right2,... */
   set<uint> _coords;
public:
   set<Contig> _frags;
   void add_read_mass(float mass);

   map<int, double> _bin_weight_map; // iso -> bin_weight
   map<int, vector<pair<int,float>>> _iso_2_frag_lens;
   ExonBin(set<uint> coordinates);
   uint left_most() const;
   uint right_most() const;
   float read_count() const;
   void add_frag_len(const int iso, const int frag_len, const float mass);
   double read_depth() const;
   int bin_len() const;
   vector<uint> bin_under_iso(const Isoform& iso,
         vector<pair<uint, uint>> & exon_coords) const;

   int effective_len(const vector<uint> & exons,
         const vector<uint>& implicit_idx,
         const int fl,
         const int rl
         ) const;
   bool operator==(const ExonBin& rhs) const;
   bool add_frag(const Contig& fg);
   int num_exons() const;
   int left_exon_len() const;
};


class Estimation {
//typedef CGAL::Quadratic_program<double> Program;
//typedef CGAL::Quadratic_program_solution<ET> Solution;

   shared_ptr<InsertSize> _insert_size_dist;
   int _read_len;

   void set_maps(const int& iso_id, const int& fg_len,
                const float& mass,
                const Contig& read,
                const set<uint>& coords,
                map<set<uint>, ExonBin> & exon_bin_map,
                map<int, set<set<uint>>> &iso_2_bins_map);

public:
   Estimation(shared_ptr<InsertSize> insert_size, int read_len);
   void test();
   void overlap_exons(const vector<GenomicFeature>& exons,
                     const Contig& read,
                     set<uint> &coords);

   void assign_exon_bin(
      const vector<Contig> &hits,
      const vector<Isoform> &transcripts,
      const vector<GenomicFeature> & exon_segs,
      map<set<uint>, ExonBin> & exon_bin_map,
      map<int, set<set<uint>>> &iso_2_bins_map);

   void theory_bin_weight(const map<int, set<set<uint>>> &iso_2_bins_map,
                          const map<int,int> &iso_2_len_map,
                          const vector<Isoform>& isoforms,
                          map<set<uint>, ExonBin> & exon_bin_map
                          );

   void empirical_bin_weight(const map<int, set<set<uint>>> &iso_2_bins_map,
                          const map<int,int> &iso_2_len_map,
                          const int m,
                          map<set<uint>, ExonBin> & exon_bin_map);

   void calculate_raw_iso_counts(const map<int, set<set<uint>>> &iso_2_bins_map,
          const map<set<uint>, ExonBin> & exon_bin_map);

   bool estimate_abundances(const map<set<uint>, ExonBin> & exon_bin_map,
                     const double mass,
                     map<int, int>& iso_2_len_map,
                     vector<Isoform>& isoforms,
                     bool with_bias_correction = true);
};





class EmSolver{
   const double TOLERANCE = std::numeric_limits<double>::denorm_min();
   int _num_isoforms;
   vector<double> _theta_after_zero;
   vector<int> _u; // observed data vector
   vector<vector<double>> _F; // hidden model matrix
   vector<vector<double>> _U; // hidden unobserved data matrix.
   int _max_iter_num = 1000;
   double _theta_limit = 1e-7;
   double _theta_change_limit = 1e-7;
public:
   vector<double> _theta;
   EmSolver( const int num_iso,
         const vector<int> &count,
         const vector<vector<double>> &model);
   bool run();
};

int gap_ef(const int l_left, const int l_right, const int l_int, const int rl, const int gap);
int no_gap_ef(const int l_left, const int l_right, const int l_int, const int fl);
uint generate_pair_end(const Contig& ct, const Contig& s, int span,  SingleOrit_t orit, Contig & mp);

#endif /* QP_H_ */
