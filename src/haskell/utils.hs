module Utils
    (
        intToList,
        digitCharToInt
    ) where

import           Data.Char (digitToInt, ord)

intToList :: Int -> [Int]
intToList = map digitToInt . show

-- convert char to int
digitCharToInt :: Char -> Int
digitCharToInt c = ord c - ord '0'
