//! Licensed under the Apache License, Version 2.0
//! http://www.apache.org/licenses/LICENSE-2.0 or the MIT license
//! http://opensource.org/licenses/MIT, at your
//! option. This file may not be copied, modified, or distributed
//! except according to those terms.

mod multi_product;
#[cfg(feature = "use_std")]
pub use self::multi_product::*;

use std::fmt;
use std::mem::replace;
use std::iter::{Fuse, Peekable, FromIterator};
use std::marker::PhantomData;
use size_hint;
use fold;

macro_rules! clone_fields {
    ($name:ident, $base:expr, $($field:ident),+) => (
        $name {
            $(
                $field : $base . $field .clone()
            ),*
        }
    );
}

/// An iterator adaptor that alternates elements from two iterators until both
/// run out.
///
/// This iterator is *fused*.
///
/// See [`.interleave()`](../trait.Itertools.html#method.interleave) for more information.
#[derive(Clone, Debug)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Interleave<I, J> {
    a: Fuse<I>,
    b: Fuse<J>,
    flag: bool,
}

/// Create an iterator that interleaves elements in `i` and `j`.
///
/// `IntoIterator` enabled version of `i.interleave(j)`.
///
/// ```
/// use itertools::interleave;
///
/// for elt in interleave(&[1, 2, 3], &[2, 3, 4]) {
///     /* loop body */
/// }
/// ```
pub fn interleave<I, J>(i: I, j: J) -> Interleave<<I as IntoIterator>::IntoIter, <J as IntoIterator>::IntoIter>
    where I: IntoIterator,
          J: IntoIterator<Item = I::Item>
{
    Interleave {
        a: i.into_iter().fuse(),
        b: j.into_iter().fuse(),
        flag: false,
    }
}

impl<I, J> Iterator for Interleave<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    type Item = I::Item;
    #[inline]
    fn next(&mut self) -> Option<I::Item> {
        self.flag = !self.flag;
        if self.flag {
            match self.a.next() {
                None => self.b.next(),
                r => r,
            }
        } else {
            match self.b.next() {
                None => self.a.next(),
                r => r,
            }
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        size_hint::add(self.a.size_hint(), self.b.size_hint())
    }
}

/// An iterator adaptor that alternates elements from the two iterators until
/// one of them runs out.
///
/// This iterator is *fused*.
///
/// See [`.interleave_shortest()`](../trait.Itertools.html#method.interleave_shortest)
/// for more information.
#[derive(Clone, Debug)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct InterleaveShortest<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    it0: I,
    it1: J,
    phase: bool, // false ==> it0, true ==> it1
}

/// Create a new `InterleaveShortest` iterator.
pub fn interleave_shortest<I, J>(a: I, b: J) -> InterleaveShortest<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    InterleaveShortest {
        it0: a,
        it1: b,
        phase: false,
    }
}

impl<I, J> Iterator for InterleaveShortest<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    type Item = I::Item;

    #[inline]
    fn next(&mut self) -> Option<I::Item> {
        match self.phase {
            false => match self.it0.next() {
                None => None,
                e => {
                    self.phase = true;
                    e
                }
            },
            true => match self.it1.next() {
                None => None,
                e => {
                    self.phase = false;
                    e
                }
            },
        }
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        let (curr_hint, next_hint) = {
            let it0_hint = self.it0.size_hint();
            let it1_hint = self.it1.size_hint();
            if self.phase {
                (it1_hint, it0_hint)
            } else {
                (it0_hint, it1_hint)
            }
        };
        let (curr_lower, curr_upper) = curr_hint;
        let (next_lower, next_upper) = next_hint;
        let (combined_lower, combined_upper) =
            size_hint::mul_scalar(size_hint::min(curr_hint, next_hint), 2);
        let lower =
            if curr_lower > next_lower {
                combined_lower + 1
            } else {
                combined_lower
            };
        let upper = {
            let extra_elem = match (curr_upper, next_upper) {
                (_, None) => false,
                (None, Some(_)) => true,
                (Some(curr_max), Some(next_max)) => curr_max > next_max,
            };
            if extra_elem {
                combined_upper.and_then(|x| x.checked_add(1))
            } else {
                combined_upper
            }
        };
        (lower, upper)
    }
}

#[derive(Clone, Debug)]
/// An iterator adaptor that allows putting back a single
/// item to the front of the iterator.
///
/// Iterator element type is `I::Item`.
pub struct PutBack<I>
    where I: Iterator
{
    top: Option<I::Item>,
    iter: I,
}

/// Create an iterator where you can put back a single item
pub fn put_back<I>(iterable: I) -> PutBack<I::IntoIter>
    where I: IntoIterator
{
    PutBack {
        top: None,
        iter: iterable.into_iter(),
    }
}

impl<I> PutBack<I>
    where I: Iterator
{
    /// put back value `value` (builder method)
    pub fn with_value(mut self, value: I::Item) -> Self {
        self.put_back(value);
        self
    }

    /// Split the `PutBack` into its parts.
    #[inline]
    pub fn into_parts(self) -> (Option<I::Item>, I) {
        let PutBack{top, iter} = self;
        (top, iter)
    }

    /// Put back a single value to the front of the iterator.
    ///
    /// If a value is already in the put back slot, it is overwritten.
    #[inline]
    pub fn put_back(&mut self, x: I::Item) {
        self.top = Some(x)
    }
}

impl<I> Iterator for PutBack<I>
    where I: Iterator
{
    type Item = I::Item;
    #[inline]
    fn next(&mut self) -> Option<I::Item> {
        match self.top {
            None => self.iter.next(),
            ref mut some => some.take(),
        }
    }
    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        // Not ExactSizeIterator because size may be larger than usize
        size_hint::add_scalar(self.iter.size_hint(), self.top.is_some() as usize)
    }

    fn all<G>(&mut self, mut f: G) -> bool
        where G: FnMut(Self::Item) -> bool
    {
        if let Some(elt) = self.top.take() {
            if !f(elt) {
                return false;
            }
        }
        self.iter.all(f)
    }

    fn fold<Acc, G>(mut self, init: Acc, mut f: G) -> Acc
        where G: FnMut(Acc, Self::Item) -> Acc,
    {
        let mut accum = init;
        if let Some(elt) = self.top.take() {
            accum = f(accum, elt);
        }
        self.iter.fold(accum, f)
    }
}

#[derive(Debug, Clone)]
/// An iterator adaptor that iterates over the cartesian product of
/// the element sets of two iterators `I` and `J`.
///
/// Iterator element type is `(I::Item, J::Item)`.
///
/// See [`.cartesian_product()`](../trait.Itertools.html#method.cartesian_product) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Product<I, J>
    where I: Iterator
{
    a: I,
    a_cur: Option<I::Item>,
    b: J,
    b_orig: J,
}

/// Create a new cartesian product iterator
///
/// Iterator element type is `(I::Item, J::Item)`.
pub fn cartesian_product<I, J>(mut i: I, j: J) -> Product<I, J>
    where I: Iterator,
          J: Clone + Iterator,
          I::Item: Clone
{
    Product {
        a_cur: i.next(),
        a: i,
        b: j.clone(),
        b_orig: j,
    }
}


impl<I, J> Iterator for Product<I, J>
    where I: Iterator,
          J: Clone + Iterator,
          I::Item: Clone
{
    type Item = (I::Item, J::Item);
    fn next(&mut self) -> Option<(I::Item, J::Item)> {
        let elt_b = match self.b.next() {
            None => {
                self.b = self.b_orig.clone();
                match self.b.next() {
                    None => return None,
                    Some(x) => {
                        self.a_cur = self.a.next();
                        x
                    }
                }
            }
            Some(x) => x
        };
        match self.a_cur {
            None => None,
            Some(ref a) => {
                Some((a.clone(), elt_b))
            }
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let has_cur = self.a_cur.is_some() as usize;
        // Not ExactSizeIterator because size may be larger than usize
        let (b_min, b_max) = self.b.size_hint();

        // Compute a * b_orig + b for both lower and upper bound
        size_hint::add(
            size_hint::mul(self.a.size_hint(), self.b_orig.size_hint()),
            (b_min * has_cur, b_max.map(move |x| x * has_cur)))
    }

    fn fold<Acc, G>(mut self, mut accum: Acc, mut f: G) -> Acc
        where G: FnMut(Acc, Self::Item) -> Acc,
    {
        // use a split loop to handle the loose a_cur as well as avoiding to
        // clone b_orig at the end.
        if let Some(mut a) = self.a_cur.take() {
            let mut b = self.b;
            loop {
                accum = b.fold(accum, |acc, elt| f(acc, (a.clone(), elt)));

                // we can only continue iterating a if we had a first element;
                if let Some(next_a) = self.a.next() {
                    b = self.b_orig.clone();
                    a = next_a;
                } else {
                    break;
                }
            }
        }
        accum
    }
}

/// A ???meta iterator adaptor???. Its closure recives a reference to the iterator
/// and may pick off as many elements as it likes, to produce the next iterator element.
///
/// Iterator element type is *X*, if the return type of `F` is *Option\<X\>*.
///
/// See [`.batching()`](../trait.Itertools.html#method.batching) for more information.
#[derive(Clone)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Batching<I, F> {
    f: F,
    iter: I,
}

impl<I, F> fmt::Debug for Batching<I, F> where I: fmt::Debug {
    debug_fmt_fields!(Batching, iter);
}

/// Create a new Batching iterator.
pub fn batching<I, F>(iter: I, f: F) -> Batching<I, F> {
    Batching { f: f, iter: iter }
}

impl<B, F, I> Iterator for Batching<I, F>
    where I: Iterator,
          F: FnMut(&mut I) -> Option<B>
{
    type Item = B;
    #[inline]
    fn next(&mut self) -> Option<B> {
        (self.f)(&mut self.iter)
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        // No information about closue behavior
        (0, None)
    }
}

/// An iterator adaptor that steps a number elements in the base iterator
/// for each iteration.
///
/// The iterator steps by yielding the next element from the base iterator,
/// then skipping forward *n-1* elements.
///
/// See [`.step()`](../trait.Itertools.html#method.step) for more information.
#[derive(Clone, Debug)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Step<I> {
    iter: Fuse<I>,
    skip: usize,
}

/// Create a `Step` iterator.
///
/// **Panics** if the step is 0.
pub fn step<I>(iter: I, step: usize) -> Step<I>
    where I: Iterator
{
    assert!(step != 0);
    Step {
        iter: iter.fuse(),
        skip: step - 1,
    }
}

impl<I> Iterator for Step<I>
    where I: Iterator
{
    type Item = I::Item;
    #[inline]
    fn next(&mut self) -> Option<I::Item> {
        let elt = self.iter.next();
        if self.skip > 0 {
            self.iter.nth(self.skip - 1);
        }
        elt
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let (low, high) = self.iter.size_hint();
        let div = |x: usize| {
            if x == 0 {
                0
            } else {
                1 + (x - 1) / (self.skip + 1)
            }
        };
        (div(low), high.map(div))
    }
}

// known size
impl<I> ExactSizeIterator for Step<I>
    where I: ExactSizeIterator
{}


struct MergeCore<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    a: Peekable<I>,
    b: Peekable<J>,
    fused: Option<bool>,
}


impl<I, J> Clone for MergeCore<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>,
          Peekable<I>: Clone,
          Peekable<J>: Clone
{
    fn clone(&self) -> Self {
        clone_fields!(MergeCore, self, a, b, fused)
    }
}

impl<I, J> MergeCore<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    fn next_with<F>(&mut self, mut less_than: F) -> Option<I::Item>
        where F: FnMut(&I::Item, &I::Item) -> bool
    {
        let less_than = match self.fused {
            Some(lt) => lt,
            None => match (self.a.peek(), self.b.peek()) {
                (Some(a), Some(b)) => less_than(a, b),
                (Some(_), None) => {
                    self.fused = Some(true);
                    true
                }
                (None, Some(_)) => {
                    self.fused = Some(false);
                    false
                }
                (None, None) => return None,
            }
        };

        if less_than {
            self.a.next()
        } else {
            self.b.next()
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        // Not ExactSizeIterator because size may be larger than usize
        size_hint::add(self.a.size_hint(), self.b.size_hint())
    }
}

/// An iterator adaptor that merges the two base iterators in ascending order.
/// If both base iterators are sorted (ascending), the result is sorted.
///
/// Iterator element type is `I::Item`.
///
/// See [`.merge()`](../trait.Itertools.html#method.merge_by) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Merge<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    merge: MergeCore<I, J>,
}

impl<I, J> Clone for Merge<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>,
          Peekable<I>: Clone,
          Peekable<J>: Clone
{
    fn clone(&self) -> Self {
        clone_fields!(Merge, self, merge)
    }
}

impl<I, J> fmt::Debug for Merge<I, J>
    where I: Iterator + fmt::Debug, J: Iterator<Item = I::Item> + fmt::Debug,
          I::Item: fmt::Debug,
{
    debug_fmt_fields!(Merge, merge.a, merge.b);
}

/// Create an iterator that merges elements in `i` and `j`.
///
/// `IntoIterator` enabled version of `i.merge(j)`.
///
/// ```
/// use itertools::merge;
///
/// for elt in merge(&[1, 2, 3], &[2, 3, 4]) {
///     /* loop body */
/// }
/// ```
pub fn merge<I, J>(i: I, j: J) -> Merge<<I as IntoIterator>::IntoIter, <J as IntoIterator>::IntoIter>
    where I: IntoIterator,
          J: IntoIterator<Item = I::Item>,
          I::Item: PartialOrd
{
    Merge {
        merge: MergeCore {
            a: i.into_iter().peekable(),
            b: j.into_iter().peekable(),
            fused: None,
        },
    }
}

impl<I, J> Iterator for Merge<I, J>
    where I: Iterator,
          J: Iterator<Item = I::Item>,
          I::Item: PartialOrd
{
    type Item = I::Item;

    fn next(&mut self) -> Option<I::Item> {
        self.merge.next_with(|a, b| a <= b)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.merge.size_hint()
    }
}

/// An iterator adaptor that merges the two base iterators in ascending order.
/// If both base iterators are sorted (ascending), the result is sorted.
///
/// Iterator element type is `I::Item`.
///
/// See [`.merge_by()`](../trait.Itertools.html#method.merge_by) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct MergeBy<I, J, F>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    merge: MergeCore<I, J>,
    cmp: F,
}

impl<I, J, F> fmt::Debug for MergeBy<I, J, F>
    where I: Iterator + fmt::Debug, J: Iterator<Item = I::Item> + fmt::Debug,
          I::Item: fmt::Debug,
{
    debug_fmt_fields!(MergeBy, merge.a, merge.b);
}

/// Create a `MergeBy` iterator.
pub fn merge_by_new<I, J, F>(a: I, b: J, cmp: F) -> MergeBy<I, J, F>
    where I: Iterator,
          J: Iterator<Item = I::Item>
{
    MergeBy {
        merge: MergeCore {
            a: a.peekable(),
            b: b.peekable(),
            fused: None,
        },
        cmp: cmp,
    }
}

impl<I, J, F> Clone for MergeBy<I, J, F>
    where I: Iterator,
          J: Iterator<Item = I::Item>,
          Peekable<I>: Clone,
          Peekable<J>: Clone,
          F: Clone
{
    fn clone(&self) -> Self {
        clone_fields!(MergeBy, self, merge, cmp)
    }
}

impl<I, J, F> Iterator for MergeBy<I, J, F>
    where I: Iterator,
          J: Iterator<Item = I::Item>,
          F: FnMut(&I::Item, &I::Item) -> bool
{
    type Item = I::Item;

    fn next(&mut self) -> Option<I::Item> {
        self.merge.next_with(&mut self.cmp)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.merge.size_hint()
    }
}

#[derive(Clone, Debug)]
pub struct CoalesceCore<I>
    where I: Iterator
{
    iter: I,
    last: Option<I::Item>,
}

impl<I> CoalesceCore<I>
    where I: Iterator
{
    fn next_with<F>(&mut self, mut f: F) -> Option<I::Item>
        where F: FnMut(I::Item, I::Item) -> Result<I::Item, (I::Item, I::Item)>
    {
        // this fuses the iterator
        let mut last = match self.last.take() {
            None => return None,
            Some(x) => x,
        };
        for next in &mut self.iter {
            match f(last, next) {
                Ok(joined) => last = joined,
                Err((last_, next_)) => {
                    self.last = Some(next_);
                    return Some(last_);
                }
            }
        }

        Some(last)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let (low, hi) = size_hint::add_scalar(self.iter.size_hint(),
                                              self.last.is_some() as usize);
        ((low > 0) as usize, hi)
    }
}

/// An iterator adaptor that may join together adjacent elements.
///
/// See [`.coalesce()`](../trait.Itertools.html#method.coalesce) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Coalesce<I, F>
    where I: Iterator
{
    iter: CoalesceCore<I>,
    f: F,
}

impl<I: Clone, F: Clone> Clone for Coalesce<I, F>
    where I: Iterator,
          I::Item: Clone
{
    fn clone(&self) -> Self {
        clone_fields!(Coalesce, self, iter, f)
    }
}

impl<I, F> fmt::Debug for Coalesce<I, F>
    where I: Iterator + fmt::Debug,
          I::Item: fmt::Debug,
{
    debug_fmt_fields!(Coalesce, iter);
}

/// Create a new `Coalesce`.
pub fn coalesce<I, F>(mut iter: I, f: F) -> Coalesce<I, F>
    where I: Iterator
{
    Coalesce {
        iter: CoalesceCore {
            last: iter.next(),
            iter: iter,
        },
        f: f,
    }
}

impl<I, F> Iterator for Coalesce<I, F>
    where I: Iterator,
          F: FnMut(I::Item, I::Item) -> Result<I::Item, (I::Item, I::Item)>
{
    type Item = I::Item;

    fn next(&mut self) -> Option<I::Item> {
        self.iter.next_with(&mut self.f)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.iter.size_hint()
    }
}

/// An iterator adaptor that removes repeated duplicates.
///
/// See [`.dedup()`](../trait.Itertools.html#method.dedup) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Dedup<I>
    where I: Iterator
{
    iter: CoalesceCore<I>,
}

impl<I: Clone> Clone for Dedup<I>
    where I: Iterator,
          I::Item: Clone
{
    fn clone(&self) -> Self {
        clone_fields!(Dedup, self, iter)
    }
}

/// Create a new `Dedup`.
pub fn dedup<I>(mut iter: I) -> Dedup<I>
    where I: Iterator
{
    Dedup {
        iter: CoalesceCore {
            last: iter.next(),
            iter: iter,
        },
    }
}

impl<I> fmt::Debug for Dedup<I>
    where I: Iterator + fmt::Debug,
          I::Item: fmt::Debug,
{
    debug_fmt_fields!(Dedup, iter);
}

impl<I> Iterator for Dedup<I>
    where I: Iterator,
          I::Item: PartialEq
{
    type Item = I::Item;

    fn next(&mut self) -> Option<I::Item> {
        self.iter.next_with(|x, y| {
            if x == y { Ok(x) } else { Err((x, y)) }
        })
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.iter.size_hint()
    }

    fn fold<Acc, G>(self, mut accum: Acc, mut f: G) -> Acc
        where G: FnMut(Acc, Self::Item) -> Acc,
    {
        if let Some(mut last) = self.iter.last {
            accum = self.iter.iter.fold(accum, |acc, elt| {
                if elt == last {
                    acc
                } else {
                    f(acc, replace(&mut last, elt))
                }
            });
            f(accum, last)
        } else {
            accum
        }
    }
}

/// An iterator adaptor that borrows from a `Clone`-able iterator
/// to only pick off elements while the predicate returns `true`.
///
/// See [`.take_while_ref()`](../trait.Itertools.html#method.take_while_ref) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct TakeWhileRef<'a, I: 'a, F> {
    iter: &'a mut I,
    f: F,
}

impl<'a, I, F> fmt::Debug for TakeWhileRef<'a, I, F>
    where I: Iterator + fmt::Debug,
{
    debug_fmt_fields!(TakeWhileRef, iter);
}

/// Create a new `TakeWhileRef` from a reference to clonable iterator.
pub fn take_while_ref<I, F>(iter: &mut I, f: F) -> TakeWhileRef<I, F>
    where I: Iterator + Clone
{
    TakeWhileRef { iter: iter, f: f }
}

impl<'a, I, F> Iterator for TakeWhileRef<'a, I, F>
    where I: Iterator + Clone,
          F: FnMut(&I::Item) -> bool
{
    type Item = I::Item;

    fn next(&mut self) -> Option<I::Item> {
        let old = self.iter.clone();
        match self.iter.next() {
            None => None,
            Some(elt) => {
                if (self.f)(&elt) {
                    Some(elt)
                } else {
                    *self.iter = old;
                    None
                }
            }
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let (_, hi) = self.iter.size_hint();
        (0, hi)
    }
}

/// An iterator adaptor that filters `Option<A>` iterator elements
/// and produces `A`. Stops on the first `None` encountered.
///
/// See [`.while_some()`](../trait.Itertools.html#method.while_some) for more information.
#[derive(Clone, Debug)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct WhileSome<I> {
    iter: I,
}

/// Create a new `WhileSome<I>`.
pub fn while_some<I>(iter: I) -> WhileSome<I> {
    WhileSome { iter: iter }
}

impl<I, A> Iterator for WhileSome<I>
    where I: Iterator<Item = Option<A>>
{
    type Item = A;

    fn next(&mut self) -> Option<A> {
        match self.iter.next() {
            None | Some(None) => None,
            Some(elt) => elt,
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let sh = self.iter.size_hint();
        (0, sh.1)
    }
}

/// An iterator to iterate through all combinations in a `Clone`-able iterator that produces tuples
/// of a specific size.
///
/// See [`.tuple_combinations()`](../trait.Itertools.html#method.tuple_combinations) for more
/// information.
#[derive(Debug)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct TupleCombinations<I, T>
    where I: Iterator,
          T: HasCombination<I>
{
    iter: T::Combination,
    _mi: PhantomData<I>,
    _mt: PhantomData<T>
}

pub trait HasCombination<I>: Sized {
    type Combination: From<I> + Iterator<Item = Self>;
}

/// Create a new `TupleCombinations` from a clonable iterator.
pub fn tuple_combinations<T, I>(iter: I) -> TupleCombinations<I, T>
    where I: Iterator + Clone,
          I::Item: Clone,
          T: HasCombination<I>,
{
    TupleCombinations {
        iter: T::Combination::from(iter),
        _mi: PhantomData,
        _mt: PhantomData,
    }
}

impl<I, T> Iterator for TupleCombinations<I, T>
    where I: Iterator,
          T: HasCombination<I>,
{
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        self.iter.next()
    }
}

#[derive(Debug)]
pub struct Tuple1Combination<I> {
    iter: I,
}

impl<I> From<I> for Tuple1Combination<I> {
    fn from(iter: I) -> Self {
        Tuple1Combination { iter: iter }
    }
}

impl<I: Iterator> Iterator for Tuple1Combination<I> {
    type Item = (I::Item,);

    fn next(&mut self) -> Option<Self::Item> {
        self.iter.next().map(|x| (x,))
    }
}

impl<I: Iterator> HasCombination<I> for (I::Item,) {
    type Combination = Tuple1Combination<I>;
}

macro_rules! impl_tuple_combination {
    ($C:ident $P:ident ; $A:ident, $($I:ident),* ; $($X:ident)*) => (
        #[derive(Debug)]
        pub struct $C<I: Iterator> {
            item: Option<I::Item>,
            iter: I,
            c: $P<I>,
        }

        impl<I: Iterator + Clone> From<I> for $C<I> {
            fn from(mut iter: I) -> Self {
                $C {
                    item: iter.next(),
                    iter: iter.clone(),
                    c: $P::from(iter),
                }
            }
        }

        impl<I: Iterator + Clone> From<I> for $C<Fuse<I>> {
            fn from(iter: I) -> Self {
                let mut iter = iter.fuse();
                $C {
                    item: iter.next(),
                    iter: iter.clone(),
                    c: $P::from(iter),
                }
            }
        }

        impl<I, $A> Iterator for $C<I>
            where I: Iterator<Item = $A> + Clone,
                  I::Item: Clone
        {
            type Item = ($($I),*);

            fn next(&mut self) -> Option<Self::Item> {
                if let Some(($($X),*,)) = self.c.next() {
                    let z = self.item.clone().unwrap();
                    Some((z, $($X),*))
                } else {
                    self.item = self.iter.next();
                    self.item.clone().and_then(|z| {
                        self.c = $P::from(self.iter.clone());
                        self.c.next().map(|($($X),*,)| (z, $($X),*))
                    })
                }
            }
        }

        impl<I, $A> HasCombination<I> for ($($I),*)
            where I: Iterator<Item = $A> + Clone,
                  I::Item: Clone
        {
            type Combination = $C<Fuse<I>>;
        }
    )
}

impl_tuple_combination!(Tuple2Combination Tuple1Combination ; A, A, A ; a);
impl_tuple_combination!(Tuple3Combination Tuple2Combination ; A, A, A, A ; a b);
impl_tuple_combination!(Tuple4Combination Tuple3Combination ; A, A, A, A, A; a b c);


/// An iterator adapter to simply flatten a structure.
///
/// See [`.flatten()`](../trait.Itertools.html#method.flatten) for more information.
#[derive(Clone, Debug)]
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Flatten<I, J> {
    iter: I,
    front: Option<J>,
}

/// Create a new `Flatten` iterator.
pub fn flatten<I, J>(iter: I) -> Flatten<I, J> {
    Flatten {
        iter: iter,
        front: None,
    }
}

impl<I, J> Iterator for Flatten<I, J>
    where I: Iterator,
          I::Item: IntoIterator<IntoIter=J, Item=J::Item>,
          J: Iterator,
{
    type Item = J::Item;
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            if let Some(ref mut f) = self.front {
                match f.next() {
                    elt @ Some(_) => return elt,
                    None => { }
                }
            }
            if let Some(next_front) = self.iter.next() {
                self.front = Some(next_front.into_iter());
            } else {
                break;
            }
        }
        None
    }

    // special case to convert segmented iterator into consecutive loops
    fn fold<Acc, G>(self, init: Acc, mut f: G) -> Acc
        where G: FnMut(Acc, Self::Item) -> Acc,
    {
        let mut accum = init;
        if let Some(iter) = self.front {
            accum = fold(iter, accum, &mut f);
        }
        self.iter.fold(accum, move |accum, iter| fold(iter, accum, &mut f))
    }

}

/// An iterator adapter to apply a transformation within a nested `Result`.
///
/// See [`.map_results()`](../trait.Itertools.html#method.map_results) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct MapResults<I, F> {
    iter: I,
    f: F
}

/// Create a new `MapResults` iterator.
pub fn map_results<I, F, T, U, E>(iter: I, f: F) -> MapResults<I, F>
    where I: Iterator<Item = Result<T, E>>,
          F: FnMut(T) -> U,
{
    MapResults {
        iter: iter,
        f: f,
    }
}

impl<I, F, T, U, E> Iterator for MapResults<I, F>
    where I: Iterator<Item = Result<T, E>>,
          F: FnMut(T) -> U,
{
    type Item = Result<U, E>;

    fn next(&mut self) -> Option<Self::Item> {
        self.iter.next().map(|v| v.map(&mut self.f))
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.iter.size_hint()
    }

    fn fold<Acc, Fold>(self, init: Acc, mut fold_f: Fold) -> Acc
        where Fold: FnMut(Acc, Self::Item) -> Acc,
    {
        let mut f = self.f;
        self.iter.fold(init, move |acc, v| fold_f(acc, v.map(&mut f)))
    }

    fn collect<C>(self) -> C
        where C: FromIterator<Self::Item>
    {
        let mut f = self.f;
        self.iter.map(move |v| v.map(&mut f)).collect()
    }
}

/// An iterator adapter to get the positions of each element that matches a predicate.
///
/// See [`.positions()`](../trait.Itertools.html#method.positions) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Positions<I, F> {
    iter: I,
    f: F,
    count: usize,
}

/// Create a new `Positions` iterator.
pub fn positions<I, F>(iter: I, f: F) -> Positions<I, F>
    where I: Iterator,
          F: FnMut(I::Item) -> bool,
{
    Positions {
        iter: iter,
        f: f,
        count: 0
    }
}

impl<I, F> Iterator for Positions<I, F>
    where I: Iterator,
          F: FnMut(I::Item) -> bool,
{
    type Item = usize;

    fn next(&mut self) -> Option<Self::Item> {
        while let Some(v) = self.iter.next() {
            let i = self.count;
            self.count = i + 1;
            if (self.f)(v) {
                return Some(i);
            }
        }
        None
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        (0, self.iter.size_hint().1)
    }
}

impl<I, F> DoubleEndedIterator for Positions<I, F>
    where I: DoubleEndedIterator + ExactSizeIterator,
          F: FnMut(I::Item) -> bool,
{
    fn next_back(&mut self) -> Option<Self::Item> {
        while let Some(v) = self.iter.next_back() {
            if (self.f)(v) {
                return Some(self.count + self.iter.len())
            }
        }
        None
    }
}

/// An iterator adapter to apply a mutating function to each element before yielding it.
///
/// See [`.update()`](../trait.Itertools.html#method.update) for more information.
#[must_use = "iterator adaptors are lazy and do nothing unless consumed"]
pub struct Update<I, F> {
    iter: I,
    f: F,
}

/// Create a new `Update` iterator.
pub fn update<I, F>(iter: I, f: F) -> Update<I, F>
where
    I: Iterator,
    F: FnMut(&mut I::Item),
{
    Update { iter: iter, f: f }
}

impl<I, F> Iterator for Update<I, F>
where
    I: Iterator,
    F: FnMut(&mut I::Item),
{
    type Item = I::Item;

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(mut v) = self.iter.next() {
            (self.f)(&mut v);
            Some(v)
        } else {
            None
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.iter.size_hint()
    }

    fn fold<Acc, G>(self, init: Acc, mut g: G) -> Acc
        where G: FnMut(Acc, Self::Item) -> Acc,
    {
        let mut f = self.f;
        self.iter.fold(init, move |acc, mut v| { f(&mut v); g(acc, v) })
    }

    // if possible, re-use inner iterator specializations in collect
    fn collect<C>(self) -> C
        where C: FromIterator<Self::Item>
    {
        let mut f = self.f;
        self.iter.map(move |mut v| { f(&mut v); v }).collect()
    }
}

impl<I, F> ExactSizeIterator for Update<I, F>
where
    I: ExactSizeIterator,
    F: FnMut(&mut I::Item),
{}

impl<I, F> DoubleEndedIterator for Update<I, F>
where
    I: DoubleEndedIterator,
    F: FnMut(&mut I::Item),
{
    fn next_back(&mut self) -> Option<Self::Item> {
        if let Some(mut v) = self.iter.next_back() {
            (self.f)(&mut v);
            Some(v)
        } else {
            None
        }
    }
}
