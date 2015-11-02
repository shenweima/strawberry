/*
 * alignment.h
 *
 *  Created on: Nov 5, 2014
 *      Author: ruolin
 */

#ifndef STRAWB_ALIGNMENTS_H_
#define STRAWB_ALIGNMENTS_H_
#include <list>
#include <map>
#include "read.h"
#include "contig.h"
#include "gff.h"
class ClusterFactory;
using IntronMap = std::map<std::pair<uint,uint>,IntronTable>;

class HitCluster{
   friend ClusterFactory;
   uint _leftmost = UINT_MAX;
   uint _rightmost = 0;
   int _hit_for_plus_strand = 0;
   int _hit_for_minus_strand = 0;
   Strand_t _first_encounter_strand = Strand_t::StrandUnknown;
   int _id;
   RefID _ref_id = -1;
   bool _final = false; // HitCluster is finished
   double _raw_mass = 0.0;
   Strand_t _strand;
   std::unordered_map<ReadID, list<PairedHit>> _open_mates;
   std::vector<PairedHit> _hits;
   std::vector<PairedHit> _uniq_hits;
   std::vector<Contig*> _ref_mRNAs; // the actually objects are owned by ClusterFactory
   std::vector<GenomicFeature> _introns;
   std::vector<float> _dep_of_cov;
public:
   static const int _kMaxGeneLen = 1000000;
   static const int _kMaxFragPerCluster = 100000;
   static const int _kMinFold4BothStrand = 10;
   HitCluster() = default;
   RefID ref_id() const;
   void ref_id(RefID id);
   uint left() const;
   void left(uint left);
   void right(uint right);
   uint right() const;
   int size() const;
   int len() const;
   Strand_t ref_strand() const;
   Strand_t guessStrand() const;
   Strand_t strand() const{
      return _strand;
   }
   bool addHit(const PairedHit &hit);
   void setBoundaries();
   void clearOpenMates();
   bool addOpenHit(ReadHitPtr hit, bool extend_by_hit, bool extend_by_partner);
   int collapseHits();
   bool overlaps(const HitCluster& rhs) const;
   bool hasRefmRNAs() const {
      return _ref_mRNAs.size() > 0;
   }
   void addRefContig(Contig *contig);
   int numOpenMates() const{
      return _open_mates.size();
   }
   void addRawMass(double m){
      _raw_mass += m;
   }
   void subRawMass(double m){
      _raw_mass -= m;
   }
   double raw_mass() const{
      return _raw_mass;
   }
   bool see_both_strands();
};

class ClusterFactory{
   int _num_cluster = 0;
   uint _prev_pos = 0;
   RefID _prev_hit_ref_id = -1; //used to judge if sam/bam is sorted.
   uint _prev_hit_pos = 0; //used to judge if sam/bam is sorted.
   size_t _refmRNA_offset;
   bool _has_load_all_refs;
   string _current_chrom;
   void compute_doc(const uint left,
                     const uint right,
                     const vector<Contig> & hits,
                     vector<float> &exon_doc,
                     IntronMap &intron_doc,
                     uint smallOverhang);
public:
   unique_ptr<HitFactory> _hit_factory;
   unique_ptr<InsertSize> _insert_size_dist =nullptr;
   static const int _kMaxOlapDist = 50;
   std::vector<Contig> _ref_mRNAs; // sort by seq_id in reference_table
   ClusterFactory(unique_ptr<HitFactory> hit_fac):
      _refmRNA_offset(0),
      _has_load_all_refs(false),
      _hit_factory(move(hit_fac))
   {}

   bool loadRefmRNAs(vector<unique_ptr<GffSeqData>> &gseqs, RefSeqTable &rt, const char *seqFile = NULL);
   bool hasLoadRefmRNAs() const {
      return _ref_mRNAs.size() > 0;
   }
   int addRef2Cluster(HitCluster &clusterOut);
   void reset_refmRNAs();

   double next_valid_alignment(ReadHit& readin);
   double rewindHit(const ReadHit& rh);
   int nextCluster_denovo(HitCluster &clusterOut,
                           uint next_ref_start_pos = UINT_MAX,
                           RefID next_ref_start_ref=INT_MAX);

   int nextCluster_refGuide(HitCluster & clusterOut);
   void rewindReference(HitCluster &clusterOut, int num_regress);

   static void mergeClusters(HitCluster & dest, HitCluster &resource);

   //void compute_doc_4_cluster(const HitCluster & hit_cluster, vector<float> &exon_doc,
                              //map<pair<uint,uint>,IntronTable>& intron_counter, uint &small_overhang);

   void filter_intron(uint cluster_left, vector<float> &exon_doc, IntronMap& intron_counter);
   void parseClusters(FILE *f);
   void inspectCluster();
   void finalizeAndAssemble(HitCluster & cluster, FILE *f, bool calculatedFD);
};



bool hit_lt_cluster(const ReadHit& hit, const HitCluster& cluster, uint olap_radius);
bool hit_gt_cluster(const ReadHit& hit, const HitCluster& cluster, uint olap_radius);
bool hit_complete_within_cluster(const PairedHit& hit, const HitCluster& cluster, uint olap_radius);
#endif /* STRAWB_ALIGNMENTS_H_ */