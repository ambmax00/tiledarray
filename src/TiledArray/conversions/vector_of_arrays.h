//
// Created by Chong Peng on 2019-05-01.
//

#ifndef TILEDARRAY_CONVERSIONS_VECTOR_OF_ARRAYS_H_
#define TILEDARRAY_CONVERSIONS_VECTOR_OF_ARRAYS_H_

#include <tiledarray.h>

namespace TiledArray {

namespace detail {

/// @brief fuses the TRanges of a vector of Arrays into 1 TRange, with the vector index forming the first mode

/// The vector dimension will be the leading dimension, and will be blocked by 1.
/// @warning all arrays in the vector must have the same TiledRange
template <typename Tile, typename Policy>
TA::TiledRange fuse_vector_of_tranges(
        const std::vector<TA::DistArray<Tile, Policy>>& arrays,
        std::size_t block_size = 1) {
  auto array_trange = arrays[0].trange();

  /// make the new TiledRange1 for new dimension
  TA::TiledRange1 new_trange1;
  {
    std::vector<std::size_t> new_trange1_v;
    auto range_size = arrays.size();
    new_trange1_v.push_back(0);
    for (std::size_t i = block_size; i < range_size; i += block_size) {
      new_trange1_v.push_back(i);
    }
    new_trange1_v.push_back(range_size);
    new_trange1 = TA::TiledRange1(new_trange1_v.begin(), new_trange1_v.end());
  }

  /// make the new range for N+1 Array
  TA::TiledRange new_trange;
  {
    auto old_trange1s = array_trange.data();
    old_trange1s.insert(old_trange1s.begin(), new_trange1);
    new_trange = TA::TiledRange(old_trange1s.begin(), old_trange1s.end());
  }

  return new_trange;
}

/// @brief fuses the Shapes of a vector of Arrays into 1 Shape, with the vector index forming the first mode
///
/// @param arrays a vector of DistArray objects; all members of @c arrays must have the same TiledRange
/// @param trange the TiledRange of the fused @c arrays
template <typename Tile>
TA::DenseShape fuse_vector_of_shapes(
    const std::vector<TA::DistArray<Tile, TA::DensePolicy>>& arrays,
    const TA::TiledRange& trange) {
  return TA::DenseShape(1, trange);
}

/// @brief fuses the Shapes of a vector of Arrays into 1 Shape, with the vector index forming the first mode
///
/// @param[in] arrays a vector of DistArray objects; all members of @c arrays must have the same TiledRange
/// @param[in] fused_trange the TiledRange of the fused @c arrays
template <typename Tile>
TA::SparseShape<float> fuse_vector_of_shapes(
    const std::vector<TA::DistArray<Tile, TA::SparsePolicy>>& arrays,
    const TA::TiledRange& fused_trange) {
  auto first_tile_in_mode0 = *fused_trange.dim(0).begin();
  const auto block_size = first_tile_in_mode0.second - first_tile_in_mode0.first;

  std::size_t ntiles_per_array = arrays[0].trange().tiles_range().volume();
  // precompute tile volumes for later repeated use
  std::vector<size_t> tile_volumes(ntiles_per_array);
  {
    const auto& tiles_range = arrays[0].trange().tiles_range();
    for(auto && tile_idx : tiles_range) {
      const auto tile_ord = tiles_range.ordinal(tile_idx);
      tile_volumes[tile_ord] = arrays[0].trange().make_tile_range(tile_idx).volume();
    }
  }

  TA::Tensor<float> fused_tile_norms(fused_trange.tiles_range());

  // compute norms of fused tiles
  // N.B. tile norms are stored in scaled format, unscale in order to compute norms of fused tiles
  std::size_t narrays = arrays.size();
  size_t fused_tile_ord = 0;
  for (size_t vidx=0, fused_vidx=0; vidx < narrays; vidx+=block_size, ++fused_vidx) {
    // how many arrays actually constribute to this fused tile ... last fused tile may have fewer than block_size
    const auto vblk_size = (narrays - vidx) >= block_size ? block_size : narrays - vidx;
    for (size_t tile_ord = 0; tile_ord != ntiles_per_array; ++tile_ord, ++fused_tile_ord) {

      float unscaled_fused_tile_norm2 = 0;
      const auto tile_volume = tile_volumes[tile_ord];
      for (size_t v = 0, vv = vidx; v != vblk_size; ++v, ++vv) {
        const auto unscaled_tile_norm = arrays[vv].shape().data()[tile_ord] * tile_volume;
        unscaled_fused_tile_norm2 += unscaled_tile_norm*unscaled_tile_norm;
      }
      const auto fused_tile_volume = tile_volume * vblk_size;
      const auto fused_tile_norm = std::sqrt(unscaled_fused_tile_norm2) / fused_tile_volume;

      *(fused_tile_norms.data() + fused_tile_ord) = fused_tile_norm;
    }
  }

  auto fused_shapes = TA::SparseShape<float>(fused_tile_norms, fused_trange, true);

  return fused_shapes;
}



/// @brief extracts the shape of a subarray of a fused array created with fuse_vector_of_arrays

/// @param[in] fused_array a DistArray created with fuse_vector_of_arrays
/// @param[in] i the index of the subarray whose Shape will be extracted (i.e. the index of the corresponding tile of the leading dimension)
/// @param[in] split_trange TiledRange of the target subarray objct
/// @return the Shape of the @c i -th subarray
template <typename Tile>
TA::DenseShape subshape_from_fused_array(
    const TA::DistArray<Tile, TA::DensePolicy>& fused_array,
    const std::size_t i,
    const TA::TiledRange& split_trange) {
  return TA::DenseShape(1, split_trange);
}

/// @brief extracts the shape of a subarray of a fused array created with fuse_vector_of_arrays

/// @param[in] fused_array a DistArray created with fuse_vector_of_arrays
/// @param[in] i the index of the subarray whose Shape will be extracted
///            (i.e. the index of the corresponding *element* index of the
///            leading dimension)
/// @param[in] split_trange TiledRange of the target subarray object
/// @return the Shape of the @c i -th subarray
template <typename Tile>
TA::SparseShape<float> subshape_from_fused_array(
    const TA::DistArray<Tile, TA::SparsePolicy>& fused_array,
    const std::size_t i,
    const TA::TiledRange& split_trange) {
  TA_ASSERT(i < fused_array.trange().dim(0).extent());

  std::size_t split_array_ntiles = split_trange.tiles_range().volume();

  TA::Tensor<float> split_tile_norms(split_trange.tiles_range());

  // map element i to its tile index
  const auto tile_idx_of_i = fused_array.trange().dim(0).element_to_tile(i);
  std::size_t offset = tile_idx_of_i * split_array_ntiles;
  const auto tile_of_i = fused_array.trange().dim(0).tile(tile_idx_of_i);
  const float extent_of_tile_of_i = tile_of_i.second - tile_of_i.first;
  auto& shape = fused_array.shape();

  // note that unlike fusion we cannot compute exact norm of the split tile
  // to guarantee upper bound we have to multiply the norms by the number of
  // split tiles in the fused tile; to see why multiplication is necessary think
  // of a tile obtained by fusing 1 nonzero tile with one or more zero tiles.
  const auto* split_tile_begin = shape.data().data() + offset;
  std::transform(split_tile_begin, split_tile_begin + split_array_ntiles,
                 split_tile_norms.data(),
                 [extent_of_tile_of_i](const float& elem) {
                   return elem * extent_of_tile_of_i;
                 });

  auto split_shape =
      TA::SparseShape<float>(split_tile_norms, split_trange, true);

  return split_shape;
}

}  // namespace detail

/// @brief fuses a vector of DistArray objects, each with the same TiledRange into a DistArray with 1 more dimensions

/// The leading dimension of the resulting array is the vector dimension, and will
/// be blocked by @block_size .
///
/// @param[in] arrays a vector of DistArray objects; every element of @c arrays must have the same TiledRange object
/// @param[in] block_size the block size for the "vector" dimension of the tiled range of the result
/// @return @c arrays fused into a DistArray
template <typename Tile, typename Policy>
TA::DistArray<Tile, Policy> fuse_vector_of_arrays(
    const std::vector<TA::DistArray<Tile, Policy>>& arrays,
    std::size_t block_size = 1) {
  auto& world = arrays[0].world();
  auto array_trange = arrays[0].trange();
  const auto mode0_extent = arrays.size();

  // make fused tiledrange
  auto fused_trange = detail::fuse_vector_of_tranges(arrays, block_size);
  std::size_t ntiles_per_array = array_trange.tiles_range().volume();

  // make fused shape
  auto fused_shape = detail::fuse_vector_of_shapes(arrays, fused_trange);

  // make fused array
  TA::DistArray<Tile, Policy> fused_array(world, fused_trange, fused_shape);

  /// copy the data from a sequence of tiles
  auto make_tile = [](const TA::Range& range, const std::vector<madness::Future<Tile>>& tiles) {
    TA_ASSERT( range.extent(0) == tiles.size() );
    Tile result(range);
    auto* result_ptr = result.data();
    for(auto&& fut_of_tile: tiles) {
      TA_ASSERT(fut_of_tile.probe());
      const auto& tile = fut_of_tile.get();
      const auto* tile_data = tile.data();
      const auto tile_volume = tile.size();
      std::copy(tile_data, tile_data + tile_volume, result_ptr);
      result_ptr += tile_volume;
    }
    return result;
  };

  /// write to blocks of fused_array
  for (auto&& fused_tile_ord : *fused_array.pmap()) {
    if (!fused_array.is_zero(fused_tile_ord)) {
      // convert ordinal of the fused tile to the ordinals of its consituent tiles
      const auto div = std::ldiv(fused_tile_ord, ntiles_per_array);
      const auto tile_idx_mode0 = div.quot;
      // ordinal of the coresponding tile in the arrays
      const auto tile_ord_array = div.rem;

      auto fused_tile_range = fused_array.trange().make_tile_range(fused_tile_ord);
      // make a vector of Futures to the input tiles
      std::vector<madness::Future<Tile>> input_tiles; input_tiles.reserve(fused_tile_range.extent(0));
      for(size_t v=0, vidx=tile_idx_mode0*block_size;
      v!=block_size && vidx<mode0_extent; ++v, ++vidx) {
        input_tiles.emplace_back(arrays[vidx].find(tile_ord_array));
      }
      fused_array.set(fused_tile_ord, world.taskq.add(
          make_tile, std::move(fused_tile_range),
          std::move(input_tiles)));
    }
  }

  return fused_array;
}


/// @brief extracts a subarray of a fused array created with fuse_vector_of_arrays

/// @param[in] fused_array a DistArray created with fuse_vector_of_arrays
/// @param[in] i the index of the subarray to be extracted
///            (i.e. the index of the corresponding *element* index of the
///            leading dimension)
/// @param[in] split_trange TiledRange of the split Array object
/// @return the @c i -th subarray
template <typename Tile, typename Policy>
TA::DistArray<Tile, Policy> subarray_from_fused_array(
    const TA::DistArray<Tile, Policy>& fused_array, std::size_t i,
    const TA::TiledRange& split_trange) {

  auto& world = fused_array.world();

  // get the shape of split Array
  auto split_shape = detail::subshape_from_fused_array(fused_array, i, split_trange);

  // determine where subtile i starts
  size_t i_offset_in_tile;
  size_t tile_idx_of_i;
  {
    tile_idx_of_i = fused_array.trange().dim(0).element_to_tile(i);
    const auto tile_of_i = fused_array.trange().dim(0).tile(tile_idx_of_i);
    TA_ASSERT(i >= tile_of_i.first && i < tile_of_i.second);
    i_offset_in_tile = i - tile_of_i.first;
  }

  // create split Array object
  TA::DistArray<Tile,Policy> split_array(world, split_trange, split_shape);

  std::size_t split_ntiles= split_trange.tiles_range().volume();

  /// copy the data from tile
  auto make_tile = [i_offset_in_tile](const TA::Range& range, const Tile& fused_tile) {
    const auto split_tile_volume = range.volume();
    return Tile(range, fused_tile.data() + i_offset_in_tile * split_tile_volume);
  };

  /// write to blocks of fused_array
  for (std::size_t index : *split_array.pmap()) {
    if (!split_array.is_zero(index)) {
      std::size_t fused_array_index = tile_idx_of_i*split_ntiles + index;

      split_array.set(index, world.taskq.add(
          make_tile, split_array.trange().make_tile_range(index),
          fused_array.find(fused_array_index)));
    }
  }

  return split_array;
}

}  // namespace TiledArray

#endif  // TILEDARRAY_CONVERSIONS_VECTOR_OF_ARRAYS_H_
