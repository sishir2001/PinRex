module Utils
    (
        intToList,
        mapWithIndex,
        arrayIsEmpty
    ) where

import Data.Char (digitToInt)
import Data.Array ( bounds, Array )

intToList :: Int -> [Int]
intToList = map digitToInt . show

mapWithIndex :: (Int -> a -> b) -> [a] -> [b]
mapWithIndex f = zipWith f [0..]


-- Check if an array is empty
arrayIsEmpty :: Array Int a -> Bool
arrayIsEmpty arr = let (start, end) = bounds arr in start > end