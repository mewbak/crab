#ifndef ABSTRACT_TRANSFORMER_HPP
#define ABSTRACT_TRANSFORMER_HPP

#include <ikos/cfg/Cfg.hpp>
#include <ikos/domains/domain_traits.hpp>

namespace analyzer
{
  using namespace cfg;
  using namespace std;

  //! API abstract transformer
  template<typename VariableName, typename AbsDomain>
  class AbsTransformer: public StatementVisitor <VariableName>
  {
   public:

    typedef AbsDomain abs_dom_t;

    typedef linear_expression< z_number, VariableName > z_lin_exp_t;

    typedef BinaryOp <z_number,VariableName>   z_bin_op_t;
    typedef Assignment <z_number,VariableName> z_assign_t;
    typedef Assume <z_number,VariableName>     z_assume_t;
    typedef Havoc<VariableName>                havoc_t;
    typedef Unreachable<VariableName>          unreach_t;
    typedef FCallSite<VariableName>            callsite_t;
    typedef Return<VariableName>               return_t;
    typedef ArrayInit<VariableName>            arr_init_t;
    typedef ArrayStore<z_number,VariableName>  z_arr_store_t;
    typedef ArrayLoad<z_number,VariableName>   z_arr_load_t;
    typedef PtrStore<z_number,VariableName>    z_ptr_store_t;
    typedef PtrLoad<z_number,VariableName>     z_ptr_load_t;
    typedef PtrAssign<z_number,VariableName>   z_ptr_assign_t;
    typedef PtrObject<VariableName>            ptr_object_t;
    typedef PtrFunction<VariableName>          ptr_function_t;

   protected: 

    virtual void exec (z_bin_op_t&)  { } 
    virtual void exec (z_assign_t&) { }
    virtual void exec (z_assume_t&) { }
    virtual void exec (havoc_t&) { }
    virtual void exec (unreach_t&) { }
    virtual void exec (callsite_t&) { }
    virtual void exec (return_t&) { }
    virtual void exec (arr_init_t&) { }
    virtual void exec (z_arr_store_t&) { }
    virtual void exec (z_arr_load_t&) { }
    virtual void exec (z_ptr_store_t&) { }
    virtual void exec (z_ptr_load_t&) { }
    virtual void exec (z_ptr_assign_t&) { }
    virtual void exec (ptr_object_t&) { }
    virtual void exec (ptr_function_t&) { }

   public: /* visitor api */

    void visit (z_bin_op_t &s) { exec (s); }
    void visit (z_assign_t &s) { exec (s); }
    void visit (z_assume_t &s) { exec (s); }
    void visit (havoc_t &s) { exec (s); }
    void visit (unreach_t &s) { exec (s); }
    void visit (callsite_t &s) { exec (s); }
    void visit (return_t &s) { exec (s); }
    void visit (arr_init_t &s) { exec (s); }
    void visit (z_arr_store_t &s) { exec (s); }
    void visit (z_arr_load_t &s) { exec (s); }
    void visit (z_ptr_store_t &s) { exec (s); }
    void visit (z_ptr_load_t &s) { exec (s); }
    void visit (z_ptr_assign_t &s) { exec (s); }
    void visit (ptr_object_t &s) { exec (s); }
    void visit (ptr_function_t &s) { exec (s); }

  };

  //! Abstract transformer specialized for a numerical abstract
  //! domain.
  template<typename VariableName, typename NumAbsDomain>
  class NumAbsTransformer: 
      public AbsTransformer <VariableName, NumAbsDomain>
  {
    typedef AbsTransformer <VariableName, NumAbsDomain> abs_transform_t;

    using typename abs_transform_t::z_lin_exp_t;
    using typename abs_transform_t::z_bin_op_t;
    using typename abs_transform_t::z_assign_t;
    using typename abs_transform_t::z_assume_t;
    using typename abs_transform_t::havoc_t;
    using typename abs_transform_t::unreach_t;
    using typename abs_transform_t::arr_init_t;
    using typename abs_transform_t::z_arr_load_t;
    using typename abs_transform_t::z_arr_store_t;

    NumAbsDomain m_inv;

   public:

    NumAbsDomain inv() const { return m_inv; }

   public:
    
    NumAbsTransformer (NumAbsDomain inv): m_inv (inv) { }
    
    void exec (z_bin_op_t& stmt) 
    {
      z_lin_exp_t op1 = stmt.left ();
      z_lin_exp_t op2 = stmt.right ();
      
      if (op1.get_variable () && op2.get_variable ())
      {
        m_inv.apply (stmt.op (), 
                     stmt.lhs ().name(), 
                     (*op1.get_variable ()).name(), 
                     (*op2.get_variable ()).name());
      }
      else
      {
        assert ( op1.get_variable () && op2.is_constant ());
        m_inv.apply (stmt.op (), 
                     stmt.lhs ().name (), 
                     (*op1.get_variable ()).name(), 
                     op2.constant ()); 
      }      
    }
    
    void exec (z_assign_t& stmt) 
    {
      m_inv.assign (stmt.lhs().name (), z_lin_exp_t (stmt.rhs()));
    }
    
    void exec (z_assume_t& stmt) 
    {
      m_inv += stmt.constraint();
    }

    void exec (havoc_t& stmt) 
    {
      m_inv -= stmt.variable();
    }

    void exec (unreach_t& stmt) 
    {
      m_inv = NumAbsDomain::bottom ();
    }

    void exec (arr_init_t &stmt) 
    {
      domain_traits::array_init (m_inv, stmt.variable ());
    }

    void exec (z_arr_store_t &stmt) 
    {
      if (stmt.index ().get_variable ())
      {
        auto arr = stmt.array ().name ();
        auto idx = *(stmt.index ().get_variable ());
        domain_traits::array_store (m_inv, 
                                    arr,
                                    idx.name(), 
                                    stmt.value (),
                                    stmt.is_singleton ());
      }
    }

    void exec (z_arr_load_t  &stmt) 
    {
      if (stmt.index ().get_variable ())
      {
        auto idx = *(stmt.index ().get_variable ());
        domain_traits::array_load (m_inv, 
                                   stmt.lhs ().name (), 
                                   stmt.array ().name (), 
                                   idx.name ());
      }
    }
  }; 

} // end namespace
#endif 
